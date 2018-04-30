#include "Evm.h"
#include "Log.h"
#include "SerialInput.h"
#include "WSPRMessageTransmitter.h"


static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;
static SerialAsyncConsoleEnhanced<10>  console;

static WSPRMessage            m;
static WSPRMessageTransmitter mt;


void setup()
{
    LogStart(9600);
    Log("Starting");

    console.RegisterCommand("c", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');
            
            Log("Setting callsign to \"", p, '"');

            m.SetCallsign((char *)p);
        }
    });

    console.RegisterCommand("g", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');
            
            Log("Setting grid to \"", p, '"');

            m.SetGrid((char *)p);
        }
    });
    
    console.RegisterCommand("p", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint8_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log("Setting power to \"", val, '"');

            m.SetPower(val);
        }
    });

    console.RegisterCommand("on", [](char *){
        Log("Radio On");
        
        mt.RadioOn();
    });
    
    console.RegisterCommand("send", [](char *){
        Log("Sending");

        mt.Send(&m);
    });
    
    console.RegisterCommand("off", [](char *){
        Log("Radio Off");

        mt.RadioOff();
    });

    console.RegisterCommand("test", [](char *){
        LogNNL("Testing settings - ");

        uint8_t retVal = mt.Test(&m);

        Log(retVal ? "OK" : "ERR");
    });


    console.Start();
    
    evm.MainLoop();
}

void loop() {}



