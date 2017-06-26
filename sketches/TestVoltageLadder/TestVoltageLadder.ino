#include "PAL.h"


void setup()
{
    // Set up pins of PORTD as output
    for (uint8_t p = 4; p < 4 + 11; ++p)
    {
        PAL.PinMode(p, OUTPUT);
    }

    PAL.PinMode(15, OUTPUT);
    
    // Staircase voltage up and down
    uint8_t i = 0;
    while (1)
    {
        PAL.DigitalToggle(15);
        
        PORTD = i;

        ++i;

        delay(1);
    }
}



void loop() {}




