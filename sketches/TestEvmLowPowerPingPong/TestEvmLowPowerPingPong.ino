#include "PAL.h"
#include "Evm.h"
#include "UtlSerial.h"


static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;
static SerialAsyncConsoleEnhanced<10>  console;


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");
    PAL.Delay(20);

    console.RegisterCommand("ping", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t delayMs  = atol(str.TokenAtIdx(1, ' '));
            Serial.print("ok ");
            Serial.println(delayMs);
            PAL.Delay(50);

            console.Stop();

            ted.SetCallback([delayMs](){
                Serial.print("pong ");
                Serial.println(delayMs);
                PAL.Delay(50);

                console.Start();
                evm.LowPowerDisable();
            });
            ted.RegisterForTimedEvent(delayMs);

            evm.LowPowerEnable();
        }
    });

    console.SetVerbose(0);
    console.Start();
    evm.LowPowerDisable();

    evm.MainLoopLowPower();
}

void loop() {}


