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
    @fix@Serial.begin(9600);
    @fix@Serial.println();
    @fix@Serial.println("Starting");
    @fix@Serial.print("Mode: ");
    @fix@Serial.print((uint8_t)PAL.GetStartupMode());
    @fix@Serial.println();

    PAL.WatchdogEnable(wt);

    for (auto ms : (uint16_t[]){ 15, 30, 60, 120, 250, 500, 1000, 2000, 4000, 8000 })
    {
        PAL.WatchdogReset();
        
        @fix@Serial.print("Sleeping for ");
        @fix@Serial.print(ms);
        @fix@Serial.print("ms");
        @fix@Serial.println();

        PAL.Delay(ms);
    }
    
    @fix@Serial.println("Looping forever");

    while (1)
    {
        // nothing to do
    }
}

void loop() {}



