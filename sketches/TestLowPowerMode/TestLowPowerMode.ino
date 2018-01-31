#include "PAL.h"

static const uint8_t PIN = 14;

void Blink(uint8_t count = 1, uint32_t delayMs = 500)
{
    PAL.PinMode(PIN, OUTPUT);
    
    for (uint8_t i = 0; i < count; ++i)
    {
        PAL.DigitalWrite(PIN, HIGH);
        PAL.Delay(delayMs);
        PAL.DigitalWrite(PIN, LOW);
        PAL.Delay(delayMs);
    }
}

void setup()
{
    Blink(2);
}

void OnWDT()
{
    Blink();
}

void GoToDeepSleep()
{
    PAL.PowerDownADC();
    PAL.PowerDownBODDuringSleep();
    PAL.SleepModePowerDown();
}

void loop()
{
    PAL.WatchdogEnableInterrupt(WatchdogTimeout::TIMEOUT_4000_MS);
    PAL.SetInterruptHandlerWDT([](){
        OnWDT();
    });
    GoToDeepSleep();
}




