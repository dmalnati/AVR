#include "PAL.h"


// The below adjustment demonstrates the synthetic continuation
// of the system clock despite deep sleep during DelaySleep.


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    while (1)
    {
        const uint32_t DELAY_MS = 2000;
        
        uint32_t timeStart = PAL.Millis();
        PAL.Delay(DELAY_MS);
        uint32_t timeAfterDelay = PAL.Millis();
        PAL.DelaySleep(DELAY_MS);
        uint32_t timeAfterDelaySleep = PAL.Millis();

        @fix@Serial.print("Time Start           : "); @fix@Serial.println(timeStart);
        @fix@Serial.print("Time After Delay     : "); @fix@Serial.println(timeAfterDelay);
        @fix@Serial.print("Time After DelaySleep: "); @fix@Serial.println(timeAfterDelaySleep);

        @fix@Serial.println();

        PAL.Delay(200);
    }
}

void loop() {}


