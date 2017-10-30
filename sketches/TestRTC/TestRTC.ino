#include "RTC.h"


static RTC rtc;


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    rtc.Sync(23, 59, 50, 0);

    while (1)
    {
        Serial.println(rtc.GetTimeAsString());

        PAL.Delay(500);
    }
}

void loop() {}


