#ifndef __SHIFT_REGISTER_H__
#define __SHIFT_REGISTER_H__


#include <stdint.h>

#include "PAL.h"


// Tested on 74HC165N (PISO)
// http://playground.arduino.cc/Code/ShiftRegSN74HC165N

class ShiftRegister
{
public:
    ShiftRegister(uint8_t pinLoad,
                  uint8_t pinClock,
                  uint8_t pinClockEnable,
                  uint8_t pinSerial)
    : pinLoad_(pinLoad)
    , pinClock_(pinClock)
    , pinClockEnable_(pinClockEnable)
    , pinSerial_(pinSerial)
    {
        Setup();
    }
    
    uint8_t ShiftIn()
    {
        uint8_t retVal = 0;
        
        // Tell the shift register that it should lock the parallel values
        PAL.DigitalWrite(pinLoad_, LOW);
        PAL.DelayMicroseconds(5);
        PAL.DigitalWrite(pinLoad_, HIGH);
        PAL.DelayMicroseconds(5);
        
        // Tell the shift register to start paying attention to the clock for
        // shifting
        PAL.DigitalWrite(pinClockEnable_, LOW);
        
        retVal = ClockInOneByte();
        
        // Tell the shift register to stop looking at the clock
        PAL.DigitalWrite(pinClockEnable_, HIGH);
        
        return retVal;
    }


private:

    uint8_t ClockInOneByte()
    {
        uint8_t retVal = 0;
        
        // Actually extract the serialized data
        // Most significant bit first
        for (uint8_t i = 0; i < 8; ++i)
        {
            PAL.DigitalWrite(pinClock_, HIGH);
            
            retVal |= (PAL.DigitalRead(pinSerial_) << (7 - i));
            
            PAL.DigitalWrite(pinClock_, LOW);
        }
        
        return retVal;
    }

    void Setup()
    {
        // Set pin input/output modes
        PAL.PinMode(pinLoad_,        OUTPUT);
        PAL.PinMode(pinClock_,       OUTPUT);
        PAL.PinMode(pinClockEnable_, OUTPUT);
        PAL.PinMode(pinSerial_,      INPUT);
        
        // Set initial state of pins
        PAL.DigitalWrite(pinClock_,       LOW);
        PAL.DigitalWrite(pinClockEnable_, HIGH);
        PAL.DigitalWrite(pinLoad_,        HIGH);
    }

    Pin pinLoad_;
    Pin pinClock_;
    Pin pinClockEnable_;
    Pin pinSerial_;
};



#endif  // __SHIFT_REGISTER_H__




