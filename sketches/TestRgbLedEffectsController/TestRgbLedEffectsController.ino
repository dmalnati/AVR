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

    console.RegisterCommand("freq", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            double val = atof(str.TokenAtIdx(1, ' '));
            uint32_t period = 1000 / val;

            Log("Freq ", val, " Hz, Period ", period, " ms");

            c.SetFrequencyAll(val);
        }
    });
    
    console.RegisterCommand("freqr", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            double val = atof(str.TokenAtIdx(1, ' '));
            uint32_t period = 1000 / val;

            Log("FreqR ", val, " Hz, PeriodR ", period, " ms");

            c.SetFrequencyRed(val);
        }
    });
    
    console.RegisterCommand("freqg", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            double val = atof(str.TokenAtIdx(1, ' '));
            uint32_t period = 1000 / val;

            Log("FreqG ", val, " Hz, PeriodG ", period, " ms");

            c.SetFrequencyGreen(val);
        }
    });
    
    console.RegisterCommand("freqb", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            double val = atof(str.TokenAtIdx(1, ' '));
            uint32_t period = 1000 / val;

            Log("FreqB ", val, " Hz, PeriodB ", period, " ms");

            c.SetFrequencyBlue(val);
        }
    });
    

    console.RegisterCommand("per", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));
            double freq = (double)1000 / val;

            Log("Period ", val, " ms, Freq ", freq, " Hz");

            c.SetPeriodAll(val);
        }
    });

    console.RegisterCommand("perr", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));
            double freq = (double)1000 / val;
            
            Log("PeriodR ", val, " ms, FreqR ", freq, " Hz");

            c.SetPeriodRed(val);
        }
    });
    
    console.RegisterCommand("perg", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));
            double freq = (double)1000 / val;
            
            Log("PeriodG ", val, " ms, FreqG ", freq, " Hz");

            c.SetPeriodGreen(val);
        }
    });
        
    console.RegisterCommand("perb", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));
            double freq = (double)1000 / val;
            
            Log("PeriodB ", val, " ms, FreqB ", freq, " Hz");

            c.SetPeriodBlue(val);
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

    evm.MainLoop();
}

void loop() {}
