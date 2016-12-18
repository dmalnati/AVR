#include "Utl.h"






void PinToggle(uint8_t pin, uint16_t delayMs)
{
    PAL.PinMode(pin, OUTPUT);
    
    PAL.DigitalWrite(pin, HIGH);
    PAL.Delay(delayMs);
    PAL.DigitalWrite(pin, LOW);
}

uint32_t GetRandomInRange(uint32_t rangeLow, uint32_t rangeHigh)
{
    uint32_t retVal = 0;

    srand(PAL.Micros());

    retVal = rangeLow + (rand() % (rangeHigh - rangeLow + 1));

    return retVal;
}



