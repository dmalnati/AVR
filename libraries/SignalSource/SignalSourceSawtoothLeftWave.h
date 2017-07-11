#ifndef __SIGNAL_SOURCE_SAWTOOTH_LEFT_WAVE_H__
#define __SIGNAL_SOURCE_SAWTOOTH_LEFT_WAVE_H__


#include "SignalSource.h"


class SignalSourceSawtoothLeftWave
: public SignalSource
{
public:
    
    inline virtual int8_t GetSample(const uint8_t brad) override
    {
        int8_t val = 127 - brad;
        
        return val;
    }

};


#endif  // __SIGNAL_SOURCE_SAWTOOTH_LEFT_WAVE_H__







