#include "Evm.h"
#include "Utl@fix@Serial.h"
#include "LedBlinker.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsole<20> shell;


static const uint8_t PIN_ONE   = 13;
static const uint8_t PIN_TWO   = 12;
static const uint8_t PIN_THREE = 11;

static LedBlinker lbOne(PIN_ONE);
static LedBlinker lbTwo(PIN_TWO);
static LedBlinker lbThree(PIN_THREE);

void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");


    lbOne.Start();

    lbTwo.SetDurationOffOn(750, 250);
    lbTwo.Start();
    
    lbThree.SetDurationOffOn(990, 10);
    lbThree.Start();

    
    shell.RegisterCommand("start", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint8_t pin = atoi(str.TokenAtIdx(1, ' '));

            if (pin == 1) { lbOne.Start();   }
            if (pin == 2) { lbTwo.Start();   }
            if (pin == 3) { lbThree.Start(); }
        }
    });

    shell.RegisterCommand("stop", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint8_t pin = atoi(str.TokenAtIdx(1, ' '));

            if (pin == 1) { lbOne.Stop();   }
            if (pin == 2) { lbTwo.Stop();   }
            if (pin == 3) { lbThree.Stop(); }
        }
    });
    
    shell.RegisterCommand("set", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 4)
        {
            uint8_t  pin           = atoi(str.TokenAtIdx(1, ' '));
            uint32_t durationOffMs = atol(str.TokenAtIdx(2, ' '));
            uint32_t durationOnMs  = atol(str.TokenAtIdx(3, ' '));

            if (pin == 1) { lbOne.SetDurationOffOn(durationOffMs, durationOnMs);   }
            if (pin == 2) { lbTwo.SetDurationOffOn(durationOffMs, durationOnMs);   }
            if (pin == 3) { lbThree.SetDurationOffOn(durationOffMs, durationOnMs); }
        }
    });

    shell.RegisterErrorHandler([&](char *cmdStr){
        @fix@Serial.print("ERR: \""); @fix@Serial.print(cmdStr); @fix@Serial.print("\"");
    });

    shell.Start();

    evm.MainLoop();
}

void loop() {}







