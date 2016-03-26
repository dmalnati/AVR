#ifndef __PIN_STATE_H__
#define __PIN_STATE_H__


#include "PAL.h"


class PinState
{
public:
    PinState() { }
    ~PinState() { }
    
    uint8_t GetPinState(uint8_t pin)
    {
        PAL.PinMode(pin, INPUT);
        
        return PAL.DigitalRead(pin);
    }
    
    void SetPinState(uint8_t pin, uint8_t value)
    {
        PAL.PinMode(pin, OUTPUT);
        
        PAL.DigitalWrite(pin, value);
    }

private:
};



#endif  // __PIN_STATE_H__



