#include "Utl.h"






void PinToggle(uint8_t pin, uint16_t delayMs)
{
    PAL.PinMode(pin, OUTPUT);
    
    PAL.DigitalWrite(pin, HIGH);
    PAL.Delay(delayMs);
    PAL.DigitalWrite(pin, LOW);
}




