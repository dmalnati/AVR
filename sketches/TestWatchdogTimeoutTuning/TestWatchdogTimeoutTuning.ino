#include "PAL.h"


volatile uint8_t flag = 0;

void OnWdtTimeout()
{
    flag = 1;
}

void setup() 
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    struct WdtData
    {
        WatchdogTimeout wt;
        uint32_t        ms;
    };

    WdtData wdtDataArr[] = {
        { WatchdogTimeout::TIMEOUT_15_MS,     15 },
        { WatchdogTimeout::TIMEOUT_30_MS,     30 },
        { WatchdogTimeout::TIMEOUT_60_MS,     60 },
        { WatchdogTimeout::TIMEOUT_120_MS,   120 },
        { WatchdogTimeout::TIMEOUT_250_MS,   250 },
        { WatchdogTimeout::TIMEOUT_500_MS,   500 },
        { WatchdogTimeout::TIMEOUT_1000_MS, 1000 },
        { WatchdogTimeout::TIMEOUT_2000_MS, 2000 },
        { WatchdogTimeout::TIMEOUT_4000_MS, 4000 },
        { WatchdogTimeout::TIMEOUT_8000_MS, 8000 },
    };
    
    while (1)
    {
        for (auto wdtData : wdtDataArr)
        {
            @fix@Serial.print(wdtData.ms);

            PAL.SetInterruptHandlerRawWDT(OnWdtTimeout);
            
            uint32_t timeStart = PAL.Millis();
            
            flag = 0;
            PAL.WatchdogEnableInterrupt(wdtData.wt);
            while (!flag) { }
            
            uint32_t timeEnd = PAL.Millis();
            uint32_t timeDiff = timeEnd - timeStart;

            @fix@Serial.print(',');
            @fix@Serial.print(timeDiff);
            @fix@Serial.println();
        }
    }
}

void loop() {}
















