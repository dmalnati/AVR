#include "PAL.h"


/*
 * Results on 8MHz:
 * 
 * Manual up/down
 * ~4.4us
 * 
 * Toggle up/down
 * ~3.8us
 * 
 * Read
 * 3.6us = 8 - 4.4
 * 
 */



void setup()
{
    Pin watchMe(15);
    Pin marker(16);

    while (1)
    {
        PAL.PinMode(watchMe, OUTPUT);
        PAL.PinMode(marker, OUTPUT);
        
        // Test changing manually the changing of values
        PAL.DigitalWrite(marker, HIGH);
        
        for (uint8_t i = 0; i < 25; ++i)
        {
            PAL.DigitalWrite(watchMe, HIGH);
            PAL.DigitalWrite(watchMe, LOW);
            
            PAL.DigitalWrite(watchMe, HIGH);
            PAL.DigitalWrite(watchMe, LOW);
            
            PAL.DigitalWrite(watchMe, HIGH);
            PAL.DigitalWrite(watchMe, LOW);
            
            PAL.DigitalWrite(watchMe, HIGH);
            PAL.DigitalWrite(watchMe, LOW);
        }

        // Test toggling of values
        PAL.DigitalWrite(marker, LOW);
        
        for (uint8_t i = 0; i < 25; ++i)
        {
            PAL.DigitalToggle(watchMe);
            PAL.DigitalToggle(watchMe);
            
            PAL.DigitalToggle(watchMe);
            PAL.DigitalToggle(watchMe);
            
            PAL.DigitalToggle(watchMe);
            PAL.DigitalToggle(watchMe);
            
            PAL.DigitalToggle(watchMe);
            PAL.DigitalToggle(watchMe);
        }

        // Test read
        PAL.PinMode(watchMe, INPUT);
        for (uint8_t i = 0; i < 25; ++i)
        {
            PAL.DigitalWrite(marker, HIGH);
            PAL.DigitalRead(watchMe);
            PAL.DigitalWrite(marker, LOW);

            PAL.DigitalWrite(marker, HIGH);
            PAL.DigitalRead(watchMe);
            PAL.DigitalWrite(marker, LOW);

            PAL.DigitalWrite(marker, HIGH);
            PAL.DigitalRead(watchMe);
            PAL.DigitalWrite(marker, LOW);

            PAL.DigitalWrite(marker, HIGH);
            PAL.DigitalRead(watchMe);
            PAL.DigitalWrite(marker, LOW);
        }
    }
}

void loop() {}











