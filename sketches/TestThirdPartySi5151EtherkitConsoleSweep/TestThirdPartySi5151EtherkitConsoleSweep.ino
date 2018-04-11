#include "PAL.h"
#include "Evm.h"
#include "UtlSerial.h"
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
    Serial.print("low  : "); Serial.println(freqLow);
    Serial.print("high : "); Serial.println(freqHigh);
    Serial.print("step : "); Serial.println(freqStep);
    Serial.print("delay: "); Serial.println(stepDelayMs);
    Serial.println();
}


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

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
        Serial.println("Start");

        freqCurr = freqLow;

        // set up callback to step up freq on schedule
        ted.SetCallback([](){
            if (freqCurr <= freqHigh)
            {
                Serial.println(freqCurr);
                si5351.set_freq((uint64_t)freqCurr * (uint64_t)100ULL, SI5351_CLK0);

                freqCurr += freqStep;
                freq = freqCurr;

                ted.RegisterForTimedEvent(stepDelayMs);
            }
            else
            {
                Serial.println("Done");
            }
        });

        // have first event fire immediately
        ted.RegisterForTimedEvent(0);

        // turn on clock
        si5351.output_enable(SI5351_CLK0, 1);
    });

    console.RegisterCommand("stop", [](char *){
        Serial.println("Stop");

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

            Serial.print("Freq now "); Serial.println(freq);
        }
    });

    console.RegisterCommand("on", [](char *){
        Serial.println("On");

        si5351.set_freq((uint64_t)freq * (uint64_t)100ULL, SI5351_CLK0);

        si5351.output_enable(SI5351_CLK0, 1);
    });    

    console.RegisterCommand("off", [](char *){
        Serial.println("Off");

        si5351.output_enable(SI5351_CLK0, 0);
    });    

    console.Start();
    
    PrintStatus();
    

    // Set up library
    si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
    si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_6MA);

    // Handle events
    evm.MainLoop();
}

void loop() {}





















