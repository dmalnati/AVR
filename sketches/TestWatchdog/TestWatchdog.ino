#include "PAL.h"


//static WatchdogTimeout wt = WatchdogTimeout::TIMEOUT_15_MS;
//static WatchdogTimeout wt = WatchdogTimeout::TIMEOUT_30_MS;
//static WatchdogTimeout wt = WatchdogTimeout::TIMEOUT_60_MS;
//static WatchdogTimeout wt = WatchdogTimeout::TIMEOUT_120_MS;
//static WatchdogTimeout wt = WatchdogTimeout::TIMEOUT_250_MS;
//static WatchdogTimeout wt = WatchdogTimeout::TIMEOUT_500_MS;
//static WatchdogTimeout wt = WatchdogTimeout::TIMEOUT_1000_MS;
//static WatchdogTimeout wt = WatchdogTimeout::TIMEOUT_2000_MS;
//static WatchdogTimeout wt = WatchdogTimeout::TIMEOUT_4000_MS;
static WatchdogTimeout wt = WatchdogTimeout::TIMEOUT_8000_MS;


void setup()
{
    Serial.begin(9600);
    Serial.println();
    Serial.println("Starting");
    Serial.print("Mode: ");
    Serial.print((uint8_t)PAL.GetStartupMode());
    Serial.println();

    PAL.WatchdogEnable(wt);

    for (auto ms : (uint16_t[]){ 15, 30, 60, 120, 250, 500, 1000, 2000, 4000, 8000 })
    {
        PAL.WatchdogReset();
        
        Serial.print("Sleeping for ");
        Serial.print(ms);
        Serial.print("ms");
        Serial.println();

        PAL.Delay(ms);
    }
    
    Serial.println("Looping forever");

    while (1)
    {
        // nothing to do
    }
}

void loop() {}



