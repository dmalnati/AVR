#include "Evm.h"
#include "Log.h"
#include "SerialInput.h"
#include "RFLink4463.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<25>  console;

static const uint8_t PIN_IRQ = 12;
static const uint8_t PIN_SDN = 13;
static const uint8_t PIN_SEL = 14;
static RFLink r(PIN_IRQ, PIN_SDN, PIN_SEL);
static RFLink4463_Raw &rr = *r.GetLinkRaw();

Pin dbg(6, LOW);


//[](RFLinkHeader *hdr, uint8_t *buf, uint8_t bufSize){
//            Log(P("RxCb - "), bufSize, P(" bytes"));
//            Log(P("  realm     : "), hdr->realm);
//            Log(P("  srcAddr   : "), hdr->srcAddr);
//            Log(P("  dstAddr   : "), hdr->dstAddr);
//            Log(P("  protocolId: "), hdr->protocolId);
//        }



static uint32_t SEND_TIME_START = 0;


void setup()
{
    LogStart(9600);
    Log("Starting");

    ///////////////////////////////////////////////////
    //
    // Shortcuts to roles you may want to test out
    //
    ///////////////////////////////////////////////////

    // raw sender - async
    console.RegisterCommand("rawsa", [](char *){
        console.Exec("somtc");
        console.Exec("sss 0");
    });

    // raw receiver
    console.RegisterCommand("rawr", [](char *){
        console.Exec("mr");
        console.Exec("somrc");
    });


    
    ///////////////////////////////////////////////////
    //
    // Raw interface
    //
    ///////////////////////////////////////////////////

    console.RegisterCommand("init", [](char *){
        Log(P("Init"));
        uint8_t retVal = r.Init();
        Log(P("  "), retVal);
    });

    console.RegisterCommand("somrc", [](char *){
        Log(P("SetOnMessageReceivedCallback(Raw)"));
        rr.SetOnMessageReceivedCallback([](uint8_t * /* buf */, uint8_t bufSize){
            Log(P("RxCbRaw - "), bufSize, P(" bytes"));
        });
    });
    console.RegisterCommand("somtc", [](char *){
        Log(P("SetOnMessageTransmittedCallback"));
        r.SetOnMessageTransmittedCallback([](){
            PAL.DigitalWrite(dbg, LOW);
            Log(P("TxCb"));
            uint32_t timeDiff = PAL.Micros() - SEND_TIME_START;
            Log(P("  Send time total: "), timeDiff, " us");
        });
    });

    console.RegisterCommand("sss", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("SetSendSync "), val);

            r.SetSendSync(val);
        }
    });

    console.RegisterCommand("mlp", [](char *){
        Log(P("ModeLowPower"));
        uint8_t retVal = r.ModeLowPower();
        Log(P("  "), retVal);
    });
    
    console.RegisterCommand("mr", [](char *){
        Log(P("ModeReceive"));
        uint8_t retVal = r.ModeReceive();
        Log(P("  "), retVal);
    });

    console.RegisterCommand("sendr", [](char *cmdStr){
        char *strSend = &cmdStr[6];
        uint8_t len = strlen(strSend);
        Log(P("Send ["), len, P("]: \""), strSend, "\"");

        SEND_TIME_START = PAL.Micros();
        PAL.DigitalWrite(dbg, HIGH);
        uint8_t retVal = rr.Send((uint8_t *)strSend, strlen(strSend));
        uint32_t timeAfterSend = PAL.Micros();
        Log(P("  "), retVal);

        uint32_t timeDiff = timeAfterSend - SEND_TIME_START;
        Log(P("  Send time: "), timeDiff, " us");
    });



    ///////////////////////////////////////////////////
    //
    // RFLink interface
    //
    ///////////////////////////////////////////////////

    








    
    
    console.RegisterCommand("phase", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Phase "), val, " brads");

            //c.SetPhaseOffsetAll(val);
        }
    });
    
    
    

    console.Start();

    uint8_t initOk = r.Init();
    Log("Init - ", initOk ? "OK" : "ERR");

    evm.MainLoop();
}

void loop() {}
