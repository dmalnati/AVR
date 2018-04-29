#include "Evm.h"
#include "Utl@fix@Serial.h"

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
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");
    
    ted.SetCallback([&](){
        @fix@Serial.println(PAL.Millis());
    });

    console.RegisterCommand("start", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t timeoutMs = atol(str.TokenAtIdx(1, ' '));

            @fix@Serial.print(F("Single timeout(")); @fix@Serial.print(PAL.Millis()); @fix@Serial.print(F("): "));
            @fix@Serial.print(timeoutMs);
            @fix@Serial.println();

            ted.RegisterForTimedEvent(timeoutMs);
        }
    });

    console.RegisterCommand("starti", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t timeoutMs = atol(str.TokenAtIdx(1, ' '));

            @fix@Serial.print(F("Interval timeout(")); @fix@Serial.print(PAL.Millis()); @fix@Serial.print(F("): "));
            @fix@Serial.print(timeoutMs);
            @fix@Serial.println();

            ted.RegisterForTimedEventInterval(timeoutMs);
        }
        else if (str.TokenCount(' ') == 3)
        {
            uint32_t timeoutMs      = atol(str.TokenAtIdx(1, ' '));
            uint32_t firstTimeoutMs = atol(str.TokenAtIdx(2, ' '));

            @fix@Serial.print(F("Interval timeout(")); @fix@Serial.print(PAL.Millis()); @fix@Serial.print(F("): "));
            @fix@Serial.print(timeoutMs);
            @fix@Serial.print(", ");
            @fix@Serial.print(firstTimeoutMs);
            @fix@Serial.println();

            ted.RegisterForTimedEventInterval(timeoutMs, firstTimeoutMs);
        }
    });

    console.RegisterCommand("stop", [](char *cmdStr){
        @fix@Serial.print(F("Stopping (")); @fix@Serial.print(PAL.Millis()); @fix@Serial.println(")");
        ted.DeRegisterForTimedEvent();
    });

    console.Start();

    evm.MainLoop();
}

void loop() {}


