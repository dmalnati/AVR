#include "Evm.h"
#include "Log.h"
#include "LogBlob.h"
#include "SerialInput.h"
#include "RFLink.h"
#include "DurationAuditor.h"


// runs at 434.000 MHz


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<5>  console;

static const uint8_t PIN_IRQ = 12;
static const uint8_t PIN_SDN = 13;
static const uint8_t PIN_SEL = 14;
static RFLink r(PIN_IRQ, PIN_SDN, PIN_SEL);
static RFLink_Raw &rr = *r.GetLinkRaw();
static RFSI4463PROPacket &radio = r.GetRadio();

static uint8_t VERBOSE = 1;

static Pin dbg(6, LOW);

static char bufTx64[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    ":)";



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
    else if (!strcmp_P(cmd, P("reinit")))
    {
        Log(P("ReInit"));
        uint8_t retVal = radio.ReInit();
        Log(P("  "), retVal);
    }
    else if (!strcmp_P(cmd, P("status")))
    {
        r.DumpStatus();
        LogNL();
    }
    else if (!strcmp_P(cmd, P("somrcr")))
    {
        Log(P("SetOnMessageReceivedCallback(Raw)"));
        rr.SetOnMessageReceivedCallback([](uint8_t *buf, uint8_t bufSize){
            PAL.DigitalToggle(dbg);
            PAL.DigitalToggle(dbg);


            RFSI4463PROPacket::Measurements m = radio.GetMeasurements();
            
            Log(P("RxCbRaw - "), bufSize, P(" bytes"));
            Log(P("RSSI: "), m.rssi);
            LogBlob(buf, bufSize);
            LogNL();
        });
    }
    else if (!strcmp_P(cmd, P("somtc")))
    {
        Log(P("SetOnMessageTransmittedCallback"));
        r.SetOnMessageTransmittedCallback([](){
            uint32_t timeNow = PAL.Micros();

            PAL.DigitalWrite(dbg, LOW);

            if (VERBOSE)
            {
                Log(P("TxCb"));
                RFSI4463PROPacket::Measurements m = radio.GetMeasurements();
                DurationAuditorMicros<5> auditor;
                auditor.Audit("TX", m.timeUsPacketTxStart);
                auditor.Audit("TXC", m.timeUsPacketTxComplete);
                auditor.Audit("TXCApp", timeNow);
                auditor.Report();
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
        rr.Send((uint8_t *)buf, sizeof(buf));
    }
    else if (!strcmp_P(cmd, P("setModeTx")))
    {
        Log(P("radio.setModeTx"));
        radio.setModeTx();
    }
    else if (!strcmp_P(cmd, P("power")))
    {
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));

            Log(P("Power "), val);

            r.SetTxPower(val);
        }
    }
    else if (!strcmp_P(cmd, P("forever")))
    {
        Log(P("Forever until reset"));
        rr.SetOnMessageTransmittedCallback([](){
            r.Send((uint8_t *)bufTx64, 4);
        });
        r.Send((uint8_t *)bufTx64, 4);
    }
    else if (!strcmp_P(cmd, P("sendr")))
    {
        const char *strSend = str.UnsafePtrAtTokenAtIdx(1, ' ');
        uint8_t len = strlen(strSend);

        // support sending a size-in-bytes as opposed to a string
        char cTmp = '\0';
        uint8_t restoreByte = 0;
        uint8_t bufSize = atoi(strSend);
        if (isdigit(strSend[0]))
        {
            strSend = bufTx64;
            len     = bufSize;

            cTmp = bufTx64[bufSize];
            bufTx64[bufSize] = '\0';

            restoreByte = 1;
        }

        Log(P("Send ["), len, P("]: \""), strSend, "\"");
        LogBlob((uint8_t *)strSend, len + 1);
        
        PAL.DigitalWrite(dbg, HIGH);
        uint8_t retVal = rr.Send((uint8_t *)strSend, len + 1);
        
        if (VERBOSE)
        {
            Log(P("  "), retVal);
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
        const char *strSend = str.UnsafePtrAtTokenAtIdx(1, ' ');
        uint8_t len = strlen(strSend);

        // support sending a size-in-bytes as opposed to a string
        char cTmp = '\0';
        uint8_t restoreByte = 0;
        uint8_t bufSize = atoi(strSend);
        if (isdigit(strSend[0]))
        {
            strSend = bufTx64;
            len     = bufSize;

            cTmp = bufTx64[bufSize];
            bufTx64[bufSize] = '\0';

            restoreByte = 1;
        }

        Log(P("Send ["), len, P("]: \""), strSend, "\"");
        LogBlob((uint8_t *)strSend, len + 1);
        
        PAL.DigitalWrite(dbg, HIGH);
        uint8_t retVal = r.Send((uint8_t *)strSend, len + 1);
        
        if (VERBOSE)
        {
            Log(P("  "), retVal);
        }
        
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

            const char *strSend = str.UnsafePtrAtTokenAtIdx(2, ' ');
            uint8_t len = strlen(strSend);

            // support sending a size-in-bytes as opposed to a string
            char cTmp = '\0';
            uint8_t restoreByte = 0;
            uint8_t bufSize = atoi(strSend);
            if (isdigit(strSend[0]))
            {
                strSend = bufTx64;
                len     = bufSize;

                cTmp = bufTx64[bufSize];
                bufTx64[bufSize] = '\0';

                restoreByte = 1;
            }

            Log(P("Send ["), len, P("]: \""), strSend, "\"");
            LogBlob((uint8_t *)strSend, len + 1);
            
            PAL.DigitalWrite(dbg, HIGH);
            uint8_t retVal = r.SendTo(dst, (uint8_t *)strSend, len + 1);
            
            if (VERBOSE)
            {
                Log(P("  "), retVal);
            }
            
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
