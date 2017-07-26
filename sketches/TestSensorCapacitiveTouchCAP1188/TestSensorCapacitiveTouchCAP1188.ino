#include "PAL.h"
#include "SensorCapacitiveTouchCAP1188.h"


static const uint8_t ADDR = 0x28;

SensorCapacitiveTouchCAP1188 cap(ADDR);


void setup()
{
    Serial.begin(9600);
    
    while (1)
    {
        Serial.println("Checking");
        
        uint8_t touched = cap.GetTouched();

        for (uint8_t i = 0; i < 8; ++i)
        {
            if ((touched << i) & 0x80)
            {
                Serial.print(i); Serial.print(" ");
            }
            else
            {
                Serial.print("  ");
            }
        }
        Serial.println();
        Serial.println();

        PAL.Delay(500);
    }
}

void loop() {}



