#ifndef __SIGNAL_SOURCE_SINE_WAVE_H__
#define __SIGNAL_SOURCE_SINE_WAVE_H__


#include <avr/pgmspace.h>

#include "FixedPoint.h"
#include "FixedPointStepper.h"


extern const int8_t SINE_TABLE[] PROGMEM;


class SignalSourceSineWave
{
    static const uint16_t SINE_TABLE_LEN = 256;
    
public:
    
    using IdxType = FixedPointStepper<Q88, SINE_TABLE_LEN>;;
    
    static inline int8_t GetSampleAtIdx(const IdxType &idx)
    {
        uint8_t  sineTableIdx    = (uint8_t)idx;
        uint16_t pgmByteLocation = (uint16_t)SINE_TABLE + sineTableIdx;
        int8_t   val             = pgm_read_byte_near(pgmByteLocation);
        
        return val;
    }

private:
    
};


#endif  // __SIGNAL_SOURCE_SINE_WAVE_H__













