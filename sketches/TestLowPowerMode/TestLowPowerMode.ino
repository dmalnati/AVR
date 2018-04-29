#include "PAL.h"


static const uint8_t PIN_BLINK = 13;
static const uint8_t PIN_TOGGLE = 15;


void Blink(uint8_t count = 1, uint32_t delayMs = 500)
{
    PAL.PinMode(PIN_BLINK, OUTPUT);
    
    for (uint8_t i = 0; i < count; ++i)
    {
        PAL.DigitalWrite(PIN_BLINK, HIGH);
        PAL.Delay(delayMs);
        PAL.DigitalWrite(PIN_BLINK, LOW);
        PAL.Delay(delayMs);
    }
}

void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");
    
    Blink(5, 100);

    Pin pinToggle(PIN_TOGGLE, LOW);
    
    while (1)
    {
        uint32_t arr[] = { 5, 15, 30, 40, 60, 120, 220, 250, 500, 777, 1000, 2000, 4000, 8000, 10000, 15000, 30000 };
        
        for (uint32_t delaySleepDurationMs : arr)
        {
            @fix@Serial.print("loop: ");  @fix@Serial.println(delaySleepDurationMs);
            
            Blink(1);

            PAL.DigitalToggle(pinToggle);
            PAL.DelayLowPower(delaySleepDurationMs);
            PAL.DigitalToggle(pinToggle);

            Blink(2, 50);
        }
    }
}

void loop()
{
}




