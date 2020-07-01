#include "Evm.h"
#include "Log.h"
#include "SerialInput.h"
#include "RgbLedPwmController.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<25>  console;
static RgbLedPwmController c;


void setup()
{
    LogStart(9600);
    Log("Starting");


    // System control
    console.RegisterCommand("start", [](char *){
        Log("Starting");
        c.Start();
    });
    
    console.RegisterCommand("stop", [](char *){
        Log("Stopping");
        c.Stop();
    });
    

    // Setters
    console.RegisterCommand("setr", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint8_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Setting red to "), val);

            c.SetRed(val);
        }
    });
    
    console.RegisterCommand("setg", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint8_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Setting green to "), val);

            c.SetGreen(val);
        }
    });

    console.RegisterCommand("setb", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint8_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Setting blue to "), val);

            c.SetBlue(val);
        }
    });


    // Getters
    console.RegisterCommand("getr", 0, [](char *){
        uint8_t val = c.GetRed();
        
        Log(P("Val of red is "), val);
    });
    console.RegisterCommand("getg", 0, [](char *){
        uint8_t val = c.GetGreen();
        
        Log(P("Val of green is "), val);
    });
    console.RegisterCommand("getb", 0, [](char *){
        uint8_t val = c.GetBlue();
        
        Log(P("Val of blue is "), val);
    });


    Log("Init");
    c.Init();

    console.Start();

    Log("MainLoop");
    evm.MainLoop();
}

void loop() {}
