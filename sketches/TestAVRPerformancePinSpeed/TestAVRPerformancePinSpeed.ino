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
 * Read Digital
 * ~1.7us = 6.1 - 4.4
 * 
 * Read Analog
 * ~113us
 * 
 * Read Analog Batch
 * ~98us
 * 
 * Read Analog Arduino
 * ~113us
 * 
 */


void setup()
{
    Pin pinDigital(15);
    Pin pinAnalog(23);
    Pin marker(16);

    uint8_t pinAnalogArduino = PAL.GetArduinoPinFromPhysicalPin(23);

    while (1)
    {
        PAL.PinMode(pinDigital, OUTPUT);
        PAL.PinMode(marker, OUTPUT);
        
        // Test changing manually the changing of values
        PAL.DigitalWrite(marker, HIGH);
        
        for (uint8_t i = 0; i < 25; ++i)
        {
            PAL.DigitalWrite(pinDigital, HIGH);
            PAL.DigitalWrite(pinDigital, LOW);
            
            PAL.DigitalWrite(pinDigital, HIGH);
            PAL.DigitalWrite(pinDigital, LOW);
            
            PAL.DigitalWrite(pinDigital, HIGH);
            PAL.DigitalWrite(pinDigital, LOW);
            
            PAL.DigitalWrite(pinDigital, HIGH);
            PAL.DigitalWrite(pinDigital, LOW);
        }

        // Test toggling of values
        PAL.DigitalWrite(marker, LOW);
        
        for (uint8_t i = 0; i < 25; ++i)
        {
            PAL.DigitalToggle(pinDigital);
            PAL.DigitalToggle(pinDigital);
            
            PAL.DigitalToggle(pinDigital);
            PAL.DigitalToggle(pinDigital);
            
            PAL.DigitalToggle(pinDigital);
            PAL.DigitalToggle(pinDigital);
            
            PAL.DigitalToggle(pinDigital);
            PAL.DigitalToggle(pinDigital);
        }

        // Test digital read
        PAL.PinMode(pinDigital, INPUT);
        for (uint8_t i = 0; i < 25; ++i)
        {
            PAL.DigitalWrite(marker, HIGH);
            PAL.DigitalRead(pinDigital);
            PAL.DigitalWrite(marker, LOW);

            PAL.DigitalWrite(marker, HIGH);
            PAL.DigitalRead(pinDigital);
            PAL.DigitalWrite(marker, LOW);

            PAL.DigitalWrite(marker, HIGH);
            PAL.DigitalRead(pinDigital);
            PAL.DigitalWrite(marker, LOW);

            PAL.DigitalWrite(marker, HIGH);
            PAL.DigitalRead(pinDigital);
            PAL.DigitalWrite(marker, LOW);
        }

        // Give a little space to separate analog from digital
        PAL.DelayMicroseconds(200);

        // Test analog read
        for (uint8_t i = 0; i < 25; ++i)
        {
            PAL.DigitalWrite(marker, HIGH);
            PAL.AnalogRead(pinAnalog);
            PAL.DigitalWrite(marker, LOW);
            
            PAL.DigitalWrite(marker, HIGH);
            PAL.AnalogRead(pinAnalog);
            PAL.DigitalWrite(marker, LOW);
            
            PAL.DigitalWrite(marker, HIGH);
            PAL.AnalogRead(pinAnalog);
            PAL.DigitalWrite(marker, LOW);
            
            PAL.DigitalWrite(marker, HIGH);
            PAL.AnalogRead(pinAnalog);
            PAL.DigitalWrite(marker, LOW);
        }

        PAL.DelayMicroseconds(200);

        // Test analog read, batch mode
        PAL.AnalogReadBatchBegin();
        for (uint8_t i = 0; i < 25; ++i)
        {
            PAL.DigitalWrite(marker, HIGH);
            PAL.AnalogRead(pinAnalog);
            PAL.DigitalWrite(marker, LOW);
            
            PAL.DigitalWrite(marker, HIGH);
            PAL.AnalogRead(pinAnalog);
            PAL.DigitalWrite(marker, LOW);
            
            PAL.DigitalWrite(marker, HIGH);
            PAL.AnalogRead(pinAnalog);
            PAL.DigitalWrite(marker, LOW);
            
            PAL.DigitalWrite(marker, HIGH);
            PAL.AnalogRead(pinAnalog);
            PAL.DigitalWrite(marker, LOW);
        }
        PAL.AnalogReadBatchEnd();

        PAL.DelayMicroseconds(200);

        // Let's see how Arduino analog functions perform
        for (uint8_t i = 0; i < 25; ++i)
        {
            PAL.DigitalWrite(marker, HIGH);
            analogRead(pinAnalogArduino);
            PAL.DigitalWrite(marker, LOW);
            
            PAL.DigitalWrite(marker, HIGH);
            analogRead(pinAnalogArduino);
            PAL.DigitalWrite(marker, LOW);
            
            PAL.DigitalWrite(marker, HIGH);
            analogRead(pinAnalogArduino);
            PAL.DigitalWrite(marker, LOW);
            
            PAL.DigitalWrite(marker, HIGH);
            analogRead(pinAnalogArduino);
            PAL.DigitalWrite(marker, LOW);
        }

        // Separate groups of readings
        PAL.Delay(50);
    }
}

void loop() {}











