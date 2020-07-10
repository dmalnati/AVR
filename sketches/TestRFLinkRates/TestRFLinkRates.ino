#include "Evm.h"
#include "Log.h"
#include "LogBlob.h"
#include "SerialInput.h"
#include "RFLink.h"
#include "DurationAuditor.h"


// runs at 434.000 MHz


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<10>  console;

static const uint8_t PIN_IRQ = 12;
static const uint8_t PIN_SDN = 13;
static const uint8_t PIN_SEL = 14;
static RFLink r(PIN_IRQ, PIN_SDN, PIN_SEL);
static RFLink_Raw &rr = *r.GetLinkRaw();
//static RFSI4463PROPacket &radio = r.GetRadio();
static TimedEventHandlerDelegate ted;


static uint32_t waitMsTx = 0;
static uint32_t waitMsRx = 0;
static uint32_t seqNo = 1;
static uint32_t seqNoExpected = 0;

static const uint8_t BUF_SIZE = 64;
static const uint8_t MSG_SIZE_MIN = 8;
static uint8_t msgSize = MSG_SIZE_MIN;

static uint32_t timeUsStart = 0;

static uint32_t msgCountSeen   = 0;
static uint32_t msgCountMissed = 0;

static void SendAndIncr()
{
    // send full sized buffers, but only the first 8 bytes are meaningful
    uint8_t buf[BUF_SIZE];
    memcpy(buf, (uint8_t *)"SEQ:", 4);
    memcpy(&buf[4], (uint8_t *)&seqNo, 4);
    
    rr.Send(buf, msgSize);
    ++seqNo;
}

void setup()
{
    LogStart(9600);
    LogNL();
    Log("Starting");

            if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_WATCHDOG)
        {
            Log(P("WDTR"));
        }
        else if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_BROWNOUT)
        {
            Log(P("BODR"));
        }
        else
        {
            Log("Other startup: ", (uint8_t)PAL.GetStartupMode());
        }


    console.RegisterCommand("show", [](char *){
        console.Exec("size");
        console.Exec("txwait");
        console.Exec("rxwait");
    });

    console.RegisterCommand("txwait", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));

            waitMsTx = val;
        }
        
        Log(P("Waiting "), waitMsTx, P(" ms between sending messages"));
        LogNL();
    });
    
    console.RegisterCommand("rxwait", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));

            waitMsRx = val;
        }

        Log(P("Waiting "), waitMsRx, P(" ms during received message processing"));
        LogNL();
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
        LogNL();
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
                        SendAndIncr();
                        LogNNL('.');
                    });
                    ted.RegisterForTimedEvent(waitMsTx);
                }
                else
                {
                    SendAndIncr();
                    LogNNL('.');
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
                Log(P("  TxWait (ms)   : "), waitMsTx);
                LogNL();

                console.Exec("listen");
            }
        });

        rr.SetOnMessageReceivedCallback([](uint8_t *, uint8_t){
            // Do nothing upon receipt
        });

        timeUsStart = PAL.Micros();
        
        SendAndIncr();
        LogNNL('.');
    });

    console.RegisterCommand("listen", [](char *){
        rr.SetOnMessageReceivedCallback([&](uint8_t *buf, uint8_t bufLen){
            uint32_t timeStart = PAL.Millis();
            
            if (bufLen >= 8 && !memcmp(buf, (uint8_t *)"SEQ:", 4))
            {
                uint32_t seqNoReceived;
                memcpy((uint8_t *)&seqNoReceived, (uint8_t *)&buf[4], 4);

                if (seqNoExpected)
                {
                    if (seqNoReceived != seqNoExpected)
                    {
                        uint32_t countMissed = (seqNoReceived - seqNoExpected);
                        
                        for (uint8_t i = 0; i < countMissed; ++i)
                        {
                            LogNNL('-');
                        }
                        
                        msgCountMissed += countMissed;
                    }

                    LogNNL('+');
                    ++msgCountSeen;
                }
                else
                {
                    Log(P("Receiving"));
                    LogNNL('+');

                    msgCountSeen = 0;
                    msgCountMissed = 0;

                    ++msgCountSeen;
                }

                seqNoExpected = seqNoReceived + 1;

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
                ted.RegisterForTimedEvent(200);
            }
        });

        rr.SetOnMessageTransmittedCallback([](){
            // There is no transmitting going on in this mode
        });

        ted.SetCallback([&](){
            seqNoExpected = 0;

            LogNL();
            Log(P("Receive complete"));
            Log(P("Report:"));
            Log(P("  Msgs seen  : "), msgCountSeen);
            Log(P("  Msgs missed: "), msgCountMissed);
            Log(P("  RxWait (ms): "), waitMsRx);
            LogNL();
        });

        r.ModeReceive();
    });

    console.SetVerbose(0);
    console.Start();
    
    uint8_t initOk = r.Init();
    Log("Init - ", initOk ? "OK" : "ERR");
    console.Exec("listen");
    LogNL();

    console.Exec("show");
    

    evm.MainLoop();
}

void loop() {}
