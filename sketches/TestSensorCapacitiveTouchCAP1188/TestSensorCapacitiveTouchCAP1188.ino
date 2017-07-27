#include "PAL.h"
#include "SensorCapacitiveTouchCAP1188.h"


static const uint8_t ADDR = 0x28;
//static const uint8_t ADDR = 0x2B;

SensorCapacitiveTouchCAP1188 cap(ADDR);


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    Pin dbg(14, LOW);

    cap.EnableLEDs();

    uint8_t touchedLast = 0;
    
    while (1)
    {
        //Serial.println("Checking");

        PAL.DigitalToggle(dbg);
        uint8_t touched = cap.GetTouched();
        PAL.DigitalToggle(dbg);

        if (touched != touchedLast)
        {
            for (uint8_t i = 0; i < 8; ++i)
            {
                if ((touched << i) & 0x80)
                {
                    Serial.print(i);
                }
                else
                {
                    Serial.print(" ");
                }
            }
            Serial.println();
        }

        touchedLast = touched;

        PAL.Delay(100);
    }
}

void loop() {}



