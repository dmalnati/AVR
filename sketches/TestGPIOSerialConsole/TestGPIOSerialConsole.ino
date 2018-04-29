#include "PAL.h"
#include "Str.h"
#include "Utl@fix@Serial.h"


static const uint8_t NUM_COMMANDS = 20;
static SerialShell<NUM_COMMANDS> shell;


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");
    @fix@Serial.println();

    shell.RegisterCommand("write", [](char *cmdStr) {
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 3)
        {
            uint8_t pin = atoi(str.TokenAtIdx(1, ' '));
            uint8_t val = atoi(str.TokenAtIdx(2, ' '));

            PAL.PinMode(pin, OUTPUT);
            
            @fix@Serial.print(F("Pin "));
            @fix@Serial.print(pin);
            @fix@Serial.print(F(" -> "));
            @fix@Serial.print(val);
            @fix@Serial.println();

            PAL.DigitalWrite(pin, val);
        }
    });

    shell.RegisterErrorHandler([](char *cmdStr) {
        @fix@Serial.print("ERR: Unrecognized \"");
        @fix@Serial.print(cmdStr);
        @fix@Serial.print("\"");
        @fix@Serial.println();
    });

    while (1)
    {
        shell.Run();
    }
}

void loop() {}

















