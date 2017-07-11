#ifndef __SIGNAL_SOURCE_SINE_WAVE_H__
#define __SIGNAL_SOURCE_SINE_WAVE_H__


#include <avr/pgmspace.h>

#include "SignalSource.h"


extern const int8_t SINE_TABLE[] PROGMEM;


class SignalSourceSineWave
: public SignalSource
{
public:
    
    inline virtual int8_t GetSample(const uint8_t brad) override
    {
        uint8_t  sineTableIdx    = brad;
        uint16_t pgmByteLocation = (uint16_t)SINE_TABLE + sineTableIdx;
        int8_t   val             = pgm_read_byte_near(pgmByteLocation);
        
        return val;
    }

};


#endif  // __SIGNAL_SOURCE_SINE_WAVE_H__













