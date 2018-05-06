#include "PAL.h"
#include "Evm.h"
#include "SerialInput.h"
#include "Log.h"
#include "si5351.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<20>  console;
static TimedEventHandlerDelegate ted;

static Si5351 si5351;


static uint32_t freq        = 10000000;
static uint32_t freqLow     = 10000000;
static uint32_t freqHigh    = 11000000;
static  int32_t freqStep    =    50000;
static uint32_t freqCurr    = freqLow;
static uint32_t stepDelayMs = 100;


void PrintStatus()
{
    Log("low  : ", freqLow);
    Log("high : ", freqHigh);
    Log("step : ", freqStep);
    Log("delay: ", stepDelayMs);
    Log();
}


void setup()
{
    LogStart(9600);
    Log("Starting");

    // Set up command interface
    console.RegisterCommand("low", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            freqLow = atol(str.TokenAtIdx(1, ' '));

            PrintStatus();
        }
    });
    
    console.RegisterCommand("high", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            freqHigh = atol(str.TokenAtIdx(1, ' '));

            PrintStatus();
        }
    });
    
    console.RegisterCommand("step", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            freqStep = atol(str.TokenAtIdx(1, ' '));

            PrintStatus();
        }
    });
    
    console.RegisterCommand("delay", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            stepDelayMs = atol(str.TokenAtIdx(1, ' '));

            PrintStatus();
        }
    });
    
    console.RegisterCommand("start", [](char *){
        Log("Start");

        freqCurr = freqLow;

        // set up callback to step up freq on schedule
        ted.SetCallback([](){
            if (freqCurr <= freqHigh)
            {
                Log(freqCurr);
                si5351.set_freq((uint64_t)freqCurr * (uint64_t)100ULL, SI5351_CLK0);

                freqCurr += freqStep;
                freq = freqCurr;

                ted.RegisterForTimedEvent(stepDelayMs);
            }
            else
            {
                Log("Done");
            }
        });

        // have first event fire immediately
        ted.RegisterForTimedEvent(0);

        // turn on clock
        si5351.output_enable(SI5351_CLK0, 1);
    });

    console.RegisterCommand("stop", [](char *){
        Log("Stop");

        // Stop processing events
        ted.DeRegisterForTimedEvent();

        // turn off clock
        si5351.output_enable(SI5351_CLK0, 0);
    });



    console.RegisterCommand("freq", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            freq = atol(str.TokenAtIdx(1, ' '));

            LogNNL("Freq now "); Log(freq);
        }
    });

    console.RegisterCommand("on", [](char *){
        Log("On");

        si5351.set_freq((uint64_t)freq * (uint64_t)100ULL, SI5351_CLK0);

        si5351.output_enable(SI5351_CLK0, 1);
    });

    console.RegisterCommand("off", [](char *){
        Log("Off");

        si5351.output_enable(SI5351_CLK0, 0);
    });


    console.RegisterCommand("drive", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint8_t mA = atoi(str.TokenAtIdx(1, ' '));

            uint8_t ok = 1;
            if      (mA == 2) { si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA); }
            else if (mA == 4) { si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_4MA); }
            else if (mA == 6) { si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_6MA); }
            else if (mA == 8) { si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); }
            else { ok = 0; }

            if (ok)
            {
                LogNNL("Drive now "); Log(mA);
            }
            else
            {
                Log("Must specify 2, 4, 6, or 8");
            }
        }
    });

    auto fnInit = [](char *){
        si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
        si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
    };
    
    console.RegisterCommand("init", fnInit);

    console.Start();
    
    PrintStatus();
    

    // Set up library
    fnInit(NULL);

    // Handle events
    evm.MainLoop();
}

void loop() {}





















