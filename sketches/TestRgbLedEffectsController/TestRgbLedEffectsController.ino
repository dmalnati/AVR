#include "Evm.h"
#include "Log.h"
#include "SerialInput.h"
#include "RgbLedEffectsController.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<25>  console;
static RgbLedEffectsControllerDebugger d;
static RgbLedEffectsController &c = d.GetController();


void setup()
{
    LogStart(9600);
    Log("Starting");


    //////////////////////////////////////////////////////////////////////
    //
    // Debugger
    //
    //////////////////////////////////////////////////////////////////////

    console.RegisterCommand("printms", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));
            
            Log(P("Printing "), val, " ms");

            d.PrintDurationMs(val);
        }
    });
    

    //////////////////////////////////////////////////////////////////////
    //
    // Controller
    //
    //////////////////////////////////////////////////////////////////////


    console.RegisterCommand("start", [](char *){
        Log("Start");
        c.Start();
    });
    console.RegisterCommand("pause", [](char *){
        Log("Pause");
        c.Pause();
    });
    console.RegisterCommand("stop", [](char *){
        Log("Stop");
        c.Stop();
    });

    console.RegisterCommand("per", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));
            double freq = (double)1000 / val;
            
            Log(P("Freq "), freq, " Hz, Period ", val, " ms");

            c.SetFrequencyAll(freq);
        }
    });

    console.RegisterCommand("perr", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));
            double freq = (double)1000 / val;
            
            Log(P("FreqR "), freq, " Hz, Period ", val, " ms");

            c.SetFrequencyRed(freq);
        }
    });
    
    console.RegisterCommand("perg", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));
            double freq = (double)1000 / val;
            
            Log(P("FreqG "), freq, " Hz, Period ", val, " ms");

            c.SetFrequencyGreen(freq);
        }
    });
        
    console.RegisterCommand("perb", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));
            double freq = (double)1000 / val;
            
            Log(P("FreqB "), freq, " Hz, Period ", val, " ms");

            c.SetFrequencyBlue(freq);
        }
    });
    
    console.RegisterCommand("phase", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Phase "), val, " brads");

            c.SetPhaseOffsetAll(val);
        }
    });
    
    console.RegisterCommand("phaser", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("PhaseR "), val, " brads");

            c.SetPhaseOffsetRed(val);
        }
    });
    
    console.RegisterCommand("phaseg", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("PhaseG "), val, " brads");

            c.SetPhaseOffsetGreen(val);
        }
    });
    
    console.RegisterCommand("phaseb", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("PhaseB "), val, " brads");

            c.SetPhaseOffsetBlue(val);
        }
    });
    
    
    

    console.Start();

    Log("MainLoop");
    evm.MainLoop();
}

void loop() {}
