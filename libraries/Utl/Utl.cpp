#include "Utl.h"






void PinToggle(uint8_t pin)
{
    PAL.PinMode(pin, OUTPUT);
    
    PAL.DigitalWrite(pin, HIGH);
    PAL.Delay(500);
    PAL.DigitalWrite(pin, LOW);
}




