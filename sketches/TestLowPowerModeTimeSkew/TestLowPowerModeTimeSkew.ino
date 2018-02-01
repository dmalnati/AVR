#include "PAL.h"


// The below adjustment demonstrates the synthetic continuation
// of the system clock despite deep sleep during DelaySleep.


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    while (1)
    {
        const uint32_t DELAY_MS = 2000;
        
        uint32_t timeStart = PAL.Millis();
        PAL.Delay(DELAY_MS);
        uint32_t timeAfterDelay = PAL.Millis();
        PAL.DelaySleep(DELAY_MS);
        uint32_t timeAfterDelaySleep = PAL.Millis();

        Serial.print("Time Start           : "); Serial.println(timeStart);
        Serial.print("Time After Delay     : "); Serial.println(timeAfterDelay);
        Serial.print("Time After DelaySleep: "); Serial.println(timeAfterDelaySleep);

        Serial.println();

        PAL.Delay(200);
    }
}

void loop() {}


