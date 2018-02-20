#include "PAL.h"
#include "Str.h"
#include "UtlSerial.h"


static const uint8_t NUM_COMMANDS = 20;
static SerialShell<NUM_COMMANDS> shell;


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");
    Serial.println();

    shell.RegisterCommand("write", [](char *cmdStr) {
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 3)
        {
            uint8_t pin = atoi(str.TokenAtIdx(1, ' '));
            uint8_t val = atoi(str.TokenAtIdx(2, ' '));

            PAL.PinMode(pin, OUTPUT);
            
            Serial.print(F("Pin "));
            Serial.print(pin);
            Serial.print(F(" -> "));
            Serial.print(val);
            Serial.println();

            PAL.DigitalWrite(pin, val);
        }
    });

    shell.RegisterErrorHandler([](char *cmdStr) {
        Serial.print("ERR: Unrecognized \"");
        Serial.print(cmdStr);
        Serial.print("\"");
        Serial.println();
    });

    while (1)
    {
        shell.Run();
    }
}

void loop() {}

















