#include "RTC.h"


static RTC rtc;


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    rtc.Sync(23, 59, 50, 0);

    while (1)
    {
        @fix@Serial.println(rtc.GetTimeAsString());

        PAL.Delay(500);
    }
}

void loop() {}


