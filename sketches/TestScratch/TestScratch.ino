#include "PAL.h"

static const uint8_t PIN = 28;

void setup()
{
    PAL.PinMode(PIN, OUTPUT);

    while (1)
    {
        PAL.DigitalToggle(PIN);

        PAL.Delay(500);
    }
}

void loop()  {}
