#ifndef __SIGNAL_SOURCE_SINE_WAVE_H__
#define __SIGNAL_SOURCE_SINE_WAVE_H__


#include <avr/pgmspace.h>


extern const int8_t SINE_TABLE[] PROGMEM;


class SignalSourceSineWave
{
public:
    
    static inline int8_t GetSample(const uint8_t brad)
    {
        uint8_t  sineTableIdx    = brad;
        uint16_t pgmByteLocation = (uint16_t)SINE_TABLE + sineTableIdx;
        int8_t   val             = pgm_read_byte_near(pgmByteLocation);
        
        return val;
    }

};


#endif  // __SIGNAL_SOURCE_SINE_WAVE_H__













