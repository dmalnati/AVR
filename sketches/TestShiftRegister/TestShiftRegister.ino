#include "ShiftRegister.h"


static const uint8_t PIN_LOAD         = 14;
static const uint8_t PIN_CLOCK        = 13;
static const uint8_t PIN_CLOCK_ENABLE = 12;
static const uint8_t PIN_SERIAL       = 11;

static ShiftRegister sr(PIN_LOAD, PIN_CLOCK, PIN_CLOCK_ENABLE, PIN_SERIAL);


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    uint8_t bitmapLast = 0;
    while (1)
    {
        uint8_t bitmap = sr.ShiftIn();

        for (uint8_t i = 0; i < 8; ++i)
        {
            uint8_t bitVal     = !!(bitmap     & (1 << (7 - i)));
            uint8_t bitValLast = !!(bitmapLast & (1 << (7 - i)));
            
            if (bitVal != bitValLast)
            {
                Serial.print("PISO Pin ");
                Serial.print(7 - i);
                Serial.print(": changed from ");
                Serial.print(bitValLast);
                Serial.print(" to ");
                Serial.print(bitVal);
                Serial.println();
            }
        }

        bitmapLast = bitmap;
    }
}


void loop() {}


















