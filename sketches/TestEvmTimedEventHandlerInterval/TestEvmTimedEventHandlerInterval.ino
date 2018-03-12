#include "Evm.h"
#include "UtlSerial.h"

/*
 * Test that interval timers work.
 * Written after adding ability to have first timeout
 * specified separately from interval.
 */

static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<10>  console;
static TimedEventHandlerDelegate ted;


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");
    
    ted.SetCallback([&](){
        Serial.println(PAL.Millis());
    });

    console.RegisterCommand("start", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t timeoutMs = atol(str.TokenAtIdx(1, ' '));

            Serial.print(F("Single timeout(")); Serial.print(PAL.Millis()); Serial.print(F("): "));
            Serial.print(timeoutMs);
            Serial.println();

            ted.RegisterForTimedEvent(timeoutMs);
        }
    });

    console.RegisterCommand("starti", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t timeoutMs = atol(str.TokenAtIdx(1, ' '));

            Serial.print(F("Interval timeout(")); Serial.print(PAL.Millis()); Serial.print(F("): "));
            Serial.print(timeoutMs);
            Serial.println();

            ted.RegisterForTimedEventInterval(timeoutMs);
        }
        else if (str.TokenCount(' ') == 3)
        {
            uint32_t timeoutMs      = atol(str.TokenAtIdx(1, ' '));
            uint32_t firstTimeoutMs = atol(str.TokenAtIdx(2, ' '));

            Serial.print(F("Interval timeout(")); Serial.print(PAL.Millis()); Serial.print(F("): "));
            Serial.print(timeoutMs);
            Serial.print(", ");
            Serial.print(firstTimeoutMs);
            Serial.println();

            ted.RegisterForTimedEventInterval(timeoutMs, firstTimeoutMs);
        }
    });

    console.RegisterCommand("stop", [](char *cmdStr){
        Serial.print(F("Stopping (")); Serial.print(PAL.Millis()); Serial.println(")");
        ted.DeRegisterForTimedEvent();
    });

    console.Start();

    evm.MainLoop();
}

void loop() {}


