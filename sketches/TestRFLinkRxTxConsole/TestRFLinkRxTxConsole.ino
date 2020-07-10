#include "Evm.h"
#include "Log.h"
#include "LogBlob.h"
#include "SerialInput.h"
#include "RFLink4463.h"


// runs at 433.500 MHz


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<5>  console;

static const uint8_t PIN_IRQ = 12;
static const uint8_t PIN_SDN = 13;
static const uint8_t PIN_SEL = 14;
static RFLink r(PIN_IRQ, PIN_SDN, PIN_SEL);
static RFLink_Raw &rr = *r.GetLinkRaw();
//static RH_RF24_mod &rf = r.GetRadio();

static uint8_t VERBOSE = 1;

static Pin dbg(6, LOW);

static char bufTx64[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    ":)";



static uint32_t SEND_TIME_START = 0;


void OnCommand(char *cmdStr)
{
    ///////////////////////////////////////////////////
    //
    // Shortcuts to roles you may want to test out
    //
    ///////////////////////////////////////////////////

    // raw sender - async
    Str str(cmdStr);
    const char *cmd = str.TokenAtIdx(0, ' ');
    
    if (!strcmp_P(cmd, P("test")))
    {
        Log("it works");

        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Val "), val);
        }
    }

    ///////////////////////////////////////////////////
    //
    // Test Role Scenario Shortcuts
    //
    ///////////////////////////////////////////////////

    // Raw Sender - Asynchronous
    else if (!strcmp_P(cmd, P("rawsa")))
    {
        console.Exec("somtc");
    }

    // Raw Receciver
    else if (!strcmp_P(cmd, P("rawr")))
    {
        console.Exec("somrcr");
        console.Exec("mr");
    }

    // Link Receciver
    else if (!strcmp_P(cmd, P("lr")))
    {
        console.Exec("somrcl");
        console.Exec("mr");
    }

    ///////////////////////////////////////////////////
    //
    // Misc
    //
    ///////////////////////////////////////////////////

    else if (!strcmp_P(cmd, P("verbose")))
    {
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));

            VERBOSE = val;
            Log(P("Verbose "), VERBOSE);
        }
    }

    
    ///////////////////////////////////////////////////
    //
    // Raw interface
    //
    ///////////////////////////////////////////////////

    else if (!strcmp_P(cmd, P("init")))
    {
        Log(P("Init"));
        uint8_t retVal = r.Init();
        Log(P("  "), retVal);
    }
    else if (!strcmp_P(cmd, P("somrcr")))
    {
        Log(P("SetOnMessageReceivedCallback(Raw)"));
        rr.SetOnMessageReceivedCallback([](uint8_t *buf, uint8_t bufSize){
            uint32_t timeDiff = PAL.Micros() - SEND_TIME_START;
            
            PAL.DigitalToggle(dbg);
            PAL.DigitalToggle(dbg);
            
            char bufReq[] = "REQ_RTT";
            char bufRep[] = "REP_RTT";

            // check if we're getting a RTT request
            if (bufSize == sizeof(bufReq))
            {
                if (!memcmp((uint8_t *)bufReq, buf, sizeof(bufReq)))
                {
                    SEND_TIME_START = PAL.Micros();
                    
                    rr.Send((uint8_t *)bufRep, sizeof(bufRep));
                    
                    Log(P("Got REQ_RTT, sent REP_RTT"));
                }
            }

            // check if we're getting a reply to a RTT
            else if (bufSize == sizeof(bufRep))
            {
                if (!memcmp((uint8_t *)bufRep, buf, sizeof(bufRep)))
                {
                    Log(P("Got REP_RTT returned"));
                    Log(P("  RTT: "), timeDiff, " us");
                }
            } 
            
            Log(P("RxCbRaw - "), bufSize, P(" bytes"));
            LogBlob(buf, bufSize);
            LogNL();
        });
    }
    else if (!strcmp_P(cmd, P("somtc")))
    {
        Log(P("SetOnMessageTransmittedCallback"));
        r.SetOnMessageTransmittedCallback([](){
            uint32_t timeDiff = PAL.Micros() - SEND_TIME_START;
            PAL.DigitalWrite(dbg, LOW);

            if (VERBOSE)
            {
                Log(P("TxCb"));
                Log(P("  Send time total: "), timeDiff, " us");
                LogNL();
            }
        });
    }
    else if (!strcmp_P(cmd, P("mr")))
    {
        Log(P("ModeReceive"));
        uint8_t retVal = r.ModeReceive();
        Log(P("  "), retVal);
    }
    else if (!strcmp_P(cmd, P("rtt")))
    {
        char buf[] = "REQ_RTT";
        
        Log(P("RTT"));
        PAL.DigitalWrite(dbg, HIGH);
        SEND_TIME_START = PAL.Micros();
        rr.Send((uint8_t *)buf, sizeof(buf));
    }
    else if (!strcmp_P(cmd, P("sendr")))
    {
        char *strSend = &cmdStr[6];
        uint8_t len = strlen(strSend);

        // support sending a size-in-bytes as opposed to a string
        uint8_t bufSize = atoi(strSend);
        char cTmp = '\0';
        uint8_t restoreByte = 0;
        if (bufSize != 0 && bufSize <= RFLink_Raw::MAX_PACKET_SIZE)
        {
            strSend = bufTx64;
            len     = bufSize;

            cTmp = bufTx64[bufSize];
            bufTx64[bufSize] = '\0';

            restoreByte = 1;
        }
        
        Log(P("Send ["), len, P("]: \""), strSend, "\"");
        LogBlob((uint8_t *)strSend, len);
        
        PAL.DigitalWrite(dbg, HIGH);
        SEND_TIME_START = PAL.Micros();
        uint8_t retVal = rr.Send((uint8_t *)strSend, len);
        
        if (VERBOSE)
        {
            uint32_t timeAfterSend = PAL.Micros();
            uint32_t timeDiff = timeAfterSend - SEND_TIME_START;
            Log(P("  "), retVal);
            Log(P("  Send time: "), timeDiff, " us");
        }
        
        if (restoreByte)
        {
            bufTx64[bufSize] = cTmp;
        }
    }

    ///////////////////////////////////////////////////
    //
    // RFLink interface
    //
    ///////////////////////////////////////////////////

    else if (!strcmp_P(cmd, P("show")))
    {
        Log(P("Realm : "), r.GetRealm());
        Log(P("Src   : "), r.GetSrcAddr());
        Log(P("Dst   : "), r.GetDstAddr());
        Log(P("Prot  : "), r.GetProtocolId());
        Log(P("RBroad: "), r.GetReceiveBroadcast());
        Log(P("PROM  : "), r.GetPromiscuousMode());
    }
    else if (!strcmp_P(cmd, P("realm")))
    {
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Realm "), val);

            r.SetRealm(val);
            console.Exec("show");
        }
    }
    else if (!strcmp_P(cmd, P("src")))
    {
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Src "), val);

            r.SetSrcAddr(val);
            console.Exec("show");
        }
    }
    else if (!strcmp_P(cmd, P("dst")))
    {
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Dst "), val);

            r.SetDstAddr(val);
            console.Exec("show");
        }
    }
    else if (!strcmp_P(cmd, P("prot")))
    {
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("ProtocolID "), val);

            r.SetProtocolId(val);
            console.Exec("show");
        }
    }
    else if (!strcmp_P(cmd, P("rbroad")))
    {
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("RcvBroadcast "), val);

            r.SetReceiveBroadcast(val);
            console.Exec("show");
        }
    }
    else if (!strcmp_P(cmd, P("prom")))
    {
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Promiscuous "), val);

            r.SetPromiscuousMode(val);
            console.Exec("show");
        }
    }
    else if (!strcmp_P(cmd, P("sendl")))
    {
        char *strSend = &cmdStr[6];
        uint8_t len = strlen(strSend);

        // support sending a size-in-bytes as opposed to a string
        uint8_t bufSize = atoi(strSend);
        char cTmp = '\0';
        uint8_t restoreByte = 0;
        if (bufSize != 0 && bufSize <= RFLink::MAX_PACKET_SIZE)
        {
            strSend = bufTx64;
            len     = bufSize;

            cTmp = bufTx64[bufSize];
            bufTx64[bufSize] = '\0';

            restoreByte = 1;
        }

        Log(P("Send ["), len, P("]: \""), strSend, "\"");
        LogBlob((uint8_t *)strSend, len);
        
        SEND_TIME_START = PAL.Micros();
        PAL.DigitalWrite(dbg, HIGH);
        uint8_t retVal = r.Send((uint8_t *)strSend, len);
        uint32_t timeAfterSend = PAL.Micros();
        Log(P("  "), retVal);

        uint32_t timeDiff = timeAfterSend - SEND_TIME_START;
        Log(P("  Send time: "), timeDiff, " us");

        if (restoreByte)
        {
            bufTx64[bufSize] = cTmp;
        }
    }
    else if (!strcmp_P(cmd, P("sendto")))
    {
        // sendto <dst> <data>
        if (str.TokenCount(' ') >= 3)
        {
            // find dst
            uint8_t dst = atoi(str.TokenAtIdx(1, ' '));

            // find data, follows command and destination
            const char *strSend = str.UnsafePtrAtTokenAtIdx(2, ' ');
            uint8_t len = strlen(strSend);
    
            // support sending a size-in-bytes as opposed to a string
            uint8_t bufSize = atoi(strSend);
            char cTmp = '\0';
            uint8_t restoreByte = 0;
            if (bufSize != 0 && bufSize <= RFLink::MAX_PACKET_SIZE)
            {
                strSend = bufTx64;
                len     = bufSize;
    
                cTmp = bufTx64[bufSize];
                bufTx64[bufSize] = '\0';
    
                restoreByte = 1;
            }
    
            Log(P("SendTo "), dst, P(" ["), len, P("]: \""), strSend, "\"");
            LogBlob((uint8_t *)strSend, len);
            
            SEND_TIME_START = PAL.Micros();
            PAL.DigitalWrite(dbg, HIGH);
            uint8_t retVal = r.SendTo(dst, (uint8_t *)strSend, len);
            uint32_t timeAfterSend = PAL.Micros();
            Log(P("  "), retVal);
    
            uint32_t timeDiff = timeAfterSend - SEND_TIME_START;
            Log(P("  Send time: "), timeDiff, " us");
    
            if (restoreByte)
            {
                bufTx64[bufSize] = cTmp;
            }
        }
    }
    else if (!strcmp_P(cmd, P("somrcl")))
    {
        Log(P("SetOnMessageReceivedCallback(Link)"));
        r.SetOnMessageReceivedCallback([](RFLinkHeader *hdr, uint8_t *buf, uint8_t bufSize){
            Log(P("RxCb - "), bufSize, P(" bytes"));
            Log(P("  realm     : "), hdr->realm);
            Log(P("  srcAddr   : "), hdr->srcAddr);
            Log(P("  dstAddr   : "), hdr->dstAddr);
            Log(P("  protocolId: "), hdr->protocolId);
            LogBlob(buf, bufSize);
            LogNL();
        });
    }


    ///////////////////////////////////////////////////
    //
    // Template
    //
    ///////////////////////////////////////////////////
   
    else if (!strcmp_P(cmd, P("")))
    {

    }



    ///////////////////////////////////////////////////
    //
    // Error
    //
    ///////////////////////////////////////////////////

    else
    {
        Log("ERR: ", cmdStr);
    }

    LogNL();
}


void setup()
{
    LogStart(9600);
    Log("Starting");

    ///////////////////////////////////////////////////
    //
    // Command Setup
    //
    ///////////////////////////////////////////////////

    console.RegisterErrorHandler([](char *cmdStr){
        OnCommand(cmdStr);
    });

    ///////////////////////////////////////////////////
    //
    // Startup
    //
    ///////////////////////////////////////////////////

    console.SetVerbose(0);
    console.Start();

    
    uint8_t initOk = r.Init();
    Log("Init - ", initOk ? "OK" : "ERR");
    LogNL();

    console.Exec("rawsa");
    console.Exec("rawr");


    evm.MainLoop();
}

void loop() {}
