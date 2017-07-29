#ifndef __MUX_ANALOG_DIGITAL_CD74HC4067_H__
#define __MUX_ANALOG_DIGITAL_CD74HC4067_H__


#include "PAL.h"


class MuxAnalogDigitalCD74HC4067
{
    static const uint8_t PROPAGATION_DELAY_TIME_US = 25;
    
public:
    MuxAnalogDigitalCD74HC4067(uint8_t pinBit0,
                               uint8_t pinBit1,
                               uint8_t pinBit2,
                               uint8_t pinBit3,
                               uint8_t pinMux)
    : pinBit0_(pinBit0)
    , pinBit1_(pinBit1)
    , pinBit2_(pinBit2)
    , pinBit3_(pinBit3)
    , pinMux_(pinMux)
    {
        PAL.PinMode(pinBit0_, OUTPUT);
        PAL.PinMode(pinBit1_, OUTPUT);
        PAL.PinMode(pinBit2_, OUTPUT);
        PAL.PinMode(pinBit3_, OUTPUT);
    }
    
    uint8_t ConnectToChannel(uint8_t channel)
    {
        uint8_t retVal = 0;
        
        // Ensure channel is limited to 4-bit resolution
        if (channel == (channel & 0b00001111))
        {
            retVal = 1;
            
            PAL.DigitalWrite(pinBit0_, (channel & 0b00000001) ? HIGH : LOW);
            PAL.DigitalWrite(pinBit1_, (channel & 0b00000010) ? HIGH : LOW);
            PAL.DigitalWrite(pinBit2_, (channel & 0b00000100) ? HIGH : LOW);
            PAL.DigitalWrite(pinBit3_, (channel & 0b00001000) ? HIGH : LOW);
            
            PAL.DelayMicroseconds(PROPAGATION_DELAY_TIME_US);
        }
        
        return retVal;
    }
    
    uint8_t GetPinMux()
    {
        return pinMux_;
    }

private:

    uint8_t pinBit0_;
    uint8_t pinBit1_;
    uint8_t pinBit2_;
    uint8_t pinBit3_;
    uint8_t pinMux_;
};



#endif  // __MUX_ANALOG_DIGITAL_CD74HC4067_H__



