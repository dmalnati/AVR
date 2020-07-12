#include "Evm.h"
#include "Log.h"
#include "LogBlob.h"
#include "SerialInput.h"
#include "RFLink.h"
#include "DurationAuditor.h"


// runs at 434.000 MHz


static Evm::Instance<5,15,10> evm;
static SerialAsyncConsoleEnhanced<15>  console;

static const uint8_t PIN_IRQ = 12;
static const uint8_t PIN_SDN = 13;
static const uint8_t PIN_SEL = 14;
static RFLink r(PIN_IRQ, PIN_SDN, PIN_SEL);
static RFLink_Raw &rr = *r.GetLinkRaw();
//static RFSI4463PROPacket &radio = r.GetRadio();
static TimedEventHandlerDelegate ted;


static uint32_t waitMsTx = 0;
static uint32_t waitMsRx = 0;
static uint32_t waitMsEnd = 1000;
static uint32_t waitMsRetry = 10;
static uint32_t seqNo = 1;

static const uint8_t BUF_SIZE = RFLink_Raw::MAX_PACKET_SIZE;
static const uint8_t MSG_SIZE_MIN = 10;
static uint8_t msgSize = MSG_SIZE_MIN;

static uint8_t txPower = 127;
static uint8_t txId[2] = { 'A', 'a' };
struct RxData
{
    uint32_t seqNo       = 0;
    uint32_t countSeen   = 0;
    uint32_t countMissed = 0;
};
const uint8_t ID_COUNT = 4;
static RxData idStart__data[ID_COUNT];

static void InitStateAll()
{
    for (uint8_t i = 0; i < ID_COUNT; ++i)
    {
        char id = 'A' + i;
        InitState(id);
    }
}

static void InitState(char idStart)
{
    if ('A' <= idStart && idStart <= ('A' + ID_COUNT))
    {
        RxData &data = idStart__data[idStart - 'A'];
        
        memset(&data, 0, sizeof(data));
    }
}

static uint32_t GetExpectedSeqNo(char idStart)
{
    uint32_t seqNo = 0;
    
    if ('A' <= idStart && idStart <= ('A' + ID_COUNT))
    {
        seqNo = idStart__data[idStart - 'A'].seqNo;
    }
    
    return seqNo;
}

static void SetExpectedSeqNo(char idStart, uint32_t seqNo)
{
    if ('A' <= idStart && idStart <= ('A' + ID_COUNT))
    {
        idStart__data[idStart - 'A'].seqNo = seqNo;
    }
}

static void IncrSeen(char idStart)
{
    if ('A' <= idStart && idStart <= ('A' + ID_COUNT))
    {
        ++idStart__data[idStart - 'A'].countSeen;
    }
}

static void IncrMissed(char idStart, uint32_t countMissed)
{
    if ('A' <= idStart && idStart <= ('A' + ID_COUNT))
    {
        idStart__data[idStart - 'A'].countMissed += countMissed;
    }
}

uint32_t GetSeen(char idStart)
{
    uint32_t retVal = 0;
    
    if ('A' <= idStart && idStart <= ('A' + ID_COUNT))
    {
        retVal = idStart__data[idStart - 'A'].countSeen;
    }

    return retVal;
}

uint32_t GetMissed(char idStart)
{
    uint32_t retVal = 0;
    
    if ('A' <= idStart && idStart <= ('A' + ID_COUNT))
    {
        retVal = idStart__data[idStart - 'A'].countMissed;
    }

    return retVal;
}

uint32_t GetSeenTotal()
{
    uint32_t retVal = 0;
    
    for (uint8_t i = 0; i < ID_COUNT; ++i)
    {
        char id = 'A' + i;
        
        retVal += GetSeen(id);
    }

    return retVal;
}

uint32_t GetMissedTotal()
{
    uint32_t retVal = 0;
    
    for (uint8_t i = 0; i < ID_COUNT; ++i)
    {
        char id = 'A' + i;
        
        retVal += GetMissed(id);
    }

    return retVal;
}

static void ReportSeen()
{
    for (uint8_t i = 0; i < ID_COUNT; ++i)
    {
        char id = 'A' + i;
        RxData &data = idStart__data[i];
        
        Log(P("    "), id, ": ", data.countSeen);
    }
}

static void ReportMissed()
{
    for (uint8_t i = 0; i < ID_COUNT; ++i)
    {
        char id = 'A' + i;
        RxData &data = idStart__data[i];
        
        Log(P("    "), id, ": ", data.countMissed);
    }
}

static uint32_t timeUsStart = 0;



static uint8_t SendAndIncr()
{
    uint8_t retVal = 0;
    
    // send full sized buffers, but only the first 8 bytes are meaningful
    uint8_t buf[BUF_SIZE];
    memcpy(buf, (uint8_t *)"SEQ:", 4);
    memcpy(&buf[4], (uint8_t *)txId, 2);
    memcpy(&buf[6], (uint8_t *)&seqNo, 4);
    
    retVal = rr.Send(buf, msgSize);

    if (retVal)
    {
        ++seqNo;
    }

    return retVal;
}

void setup()
{
    LogStart(9600);
    LogNL();
    Log("Starting rate tester");

    console.RegisterCommand("show", [](char *){
        console.Exec("size");
        console.Exec("txpower");
        console.Exec("txid");
        console.Exec("txwait");
        console.Exec("txretrywait");
        console.Exec("rxwait");
        console.Exec("endwait");
        LogNL();
        console.Exec("status");
    });

    console.RegisterCommand("status", [](char *){
        r.DumpStatus();
    });

    console.RegisterCommand("txpower", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint8_t txPowerNew = atoi(str.TokenAtIdx(1, ' '));

            if (txPowerNew <= 127)
            {
                txPower = txPowerNew;

                r.SetTxPower(txPower);
            }
            else
            {
                Log(P("txpower must be <= 127"));
            }
        }
        
        Log(P("TxPower "), txPower);
    });
    
    console.RegisterCommand("txid", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            const char *txIdNew    = str.TokenAtIdx(1, ' ');
            uint8_t     txIdNewLen = strlen(txIdNew);
            
            if (txIdNewLen == 2)
            {
                memcpy(txId, txIdNew, 2);
            }
            else
            {
                Log(P("TX ID must be 2 char, not "), txIdNewLen);
            }
        }
        
        Log(P("TX ID "), (char)txId[0], (char)txId[1]);
    });

    console.RegisterCommand("txwait", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));

            waitMsTx = val;
        }
        
        Log(P("Waiting "), waitMsTx, P(" ms between sending messages"));
    });

    console.RegisterCommand("txretrywait", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));

            waitMsRetry = val;
        }

        Log(P("Waiting "), waitMsRetry, P(" ms before retrying send"));
    });
    
    console.RegisterCommand("rxwait", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));

            waitMsRx = val;
        }

        Log(P("Waiting "), waitMsRx, P(" ms during received message processing"));
    });

    console.RegisterCommand("endwait", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));

            waitMsEnd = val;
        }

        Log(P("Waiting "), waitMsEnd, P(" ms before ending receive batch"));
    });

    console.RegisterCommand("size", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));

            if (val < MSG_SIZE_MIN || val > BUF_SIZE)
            {
                Log(P("Must set "), MSG_SIZE_MIN, P(" <= size <= "), BUF_SIZE);
                val = msgSize;
            }

            msgSize = val;
        }

        Log(P("Message size "), msgSize);
    });

    console.RegisterCommand("send", 1, [](char *cmdStr){
        Str str(cmdStr);

        uint32_t sendCount = atol(str.TokenAtIdx(1, ' '));

        seqNo = 1;
        uint32_t stopAtSeqNo = seqNo + sendCount;
        
        Log(P("Sending "), sendCount, P(" msgs"));

        ted.DeRegisterForTimedEvent();

        rr.SetOnMessageTransmittedCallback([=](){
            if (seqNo != stopAtSeqNo)
            {
                if (waitMsTx)
                {
                    ted.SetCallback([](){
                        if (SendAndIncr())
                        {
                            LogNNL('S');
                        }
                        else
                        {
                            LogNNL('s');

                            // Failed to send, so no callback, so try again
                            ted.RegisterForTimedEvent(waitMsRetry);
                        }
                    });
                    ted.RegisterForTimedEvent(waitMsTx);
                }
                else
                {
                    if (SendAndIncr())
                    {
                        LogNNL('S');
                    }
                    else
                    {
                        LogNNL('s');

                        // Failed to send, so no callback, so try again
                        ted.SetCallback(rr.GetOnMessageTransmittedCallback());
                        ted.RegisterForTimedEvent(waitMsRetry);
                    }
                }
            }
            else
            {
                uint32_t timeUsEnd = PAL.Micros();
                uint32_t timeUsDiff = timeUsEnd - timeUsStart;

                double durationSec = (double)(timeUsDiff) / 1000000;
                double bytesPerSec = ((double)msgSize * (double)sendCount) / durationSec;

                uint32_t msPerMsg = durationSec / (double)sendCount * 1000;
                
                LogNL();
                Log(P("Send complete"));
                Log(P("Report:"));
                Log(P("  Sent count    : "), sendCount, P(" msgs"));
                Log(P("  Msg size      : "), msgSize);
                Log(P("  Duration (sec): "), durationSec);
                Log(P("  Bytes/sec     : "), (uint32_t)bytesPerSec);
                Log(P("  Kbps          : "), bytesPerSec * 8.0 / 1000.0);
                Log(P("  Mbps          : "), bytesPerSec * 8.0 / 1000.0 / 1000.0);
                Log(P("  ms/msg        : "), msPerMsg);
                Log(P("  TxPower       : "), txPower);
                Log(P("  TxWait (ms)   : "), waitMsTx);
                Log(P("  TxId          : "), txId[0], txId[1]);

                console.Exec("listen");
            }
        });

        rr.SetOnMessageReceivedCallback([](uint8_t *, uint8_t){
            // Do nothing upon receipt
        });

        timeUsStart = PAL.Micros();
        
        if (SendAndIncr())
        {
            LogNNL('S');
        }
        else
        {
            LogNNL('s');
            
            // Failed to send, so no callback, so try again
            ted.SetCallback(rr.GetOnMessageTransmittedCallback());
            ted.RegisterForTimedEvent(waitMsRetry);
        }
    });

    console.RegisterCommand("listen", [](char *){
        InitStateAll();
        
        rr.SetOnMessageReceivedCallback([&](uint8_t *buf, uint8_t bufLen){
            uint32_t timeStart = PAL.Millis();
            
            if (bufLen >= 8 && !memcmp(buf, (uint8_t *)"SEQ:", 4))
            {
                uint8_t rxId[2];
                memcpy(rxId, &buf[4], 2);
                char rxIdGood = rxId[0];
                char rxIdBad  = rxId[1];
                uint32_t seqNoExpected = GetExpectedSeqNo(rxIdGood);
                
                uint32_t seqNoReceived;
                memcpy((uint8_t *)&seqNoReceived, (uint8_t *)&buf[6], 4);

                if (seqNoExpected)
                {
                    if (seqNoReceived > seqNoExpected)
                    {
                        uint32_t countMissed = (seqNoReceived - seqNoExpected);
                        
                        for (uint8_t i = 0; i < countMissed; ++i)
                        {
                            LogNNL(rxIdBad);
                        }
                        
                        IncrMissed(rxIdGood, countMissed);
                    }

                    LogNNL(rxIdGood);
                    IncrSeen(rxIdGood);
                }
                else
                {
                    LogNNL(rxIdGood);

                    InitState(rxIdGood);
                    IncrSeen(rxIdGood);
                }

                SetExpectedSeqNo(rxIdGood, seqNoReceived + 1);

                // Allow for configurable delay period to represent processing time
                // and see the effect on received/missed packets
                uint32_t timeUsed = PAL.Millis() - timeStart;
                uint32_t timeWait = 0;
                if (timeUsed < waitMsRx)
                {
                    timeWait = waitMsRx - timeUsed;
                }
                
                PAL.Delay(timeWait);

                // If no new messages seen for a while, re-set expectations
                ted.RegisterForTimedEvent(waitMsEnd);
            }
        });

        rr.SetOnMessageTransmittedCallback([](){
            // There is no transmitting going on in this mode
        });

        ted.SetCallback([&](){
            LogNL();
            Log(P("Receive complete"));
            Log(P("Report:"));
            Log(P("  Msgs seen: "), GetSeenTotal());
            ReportSeen();
            Log(P("  Msgs missed: "), GetMissedTotal());
            ReportMissed();
            Log(P("  RxWait (ms): "), waitMsRx);
            LogNL();

            InitStateAll();
        });

        r.ModeReceive();
    });

    console.SetVerbose(0);
    console.Start();
    
    uint8_t initOk = r.Init();
    Log("Init - ", initOk ? "OK" : "ERR");
    LogNL();
    r.SetTxPower(txPower);
    console.Exec("listen");
    console.Exec("show");
    LogNL();
    console.SetVerbose(1);

    evm.MainLoop();
}

void loop() {}
