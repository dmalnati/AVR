#ifndef __SIGNAL_SOURCE_SAWTOOTH_RIGHT_WAVE_H__
#define __SIGNAL_SOURCE_SAWTOOTH_RIGHT_WAVE_H__


#include "SignalSource.h"


class SignalSourceSawtoothRightWave
: public SignalSource
{
public:
    
    inline virtual int8_t GetSample(const uint8_t brad) override
    {
        int8_t val = brad - 128;
        
        return val;
    }

};


#endif  // __SIGNAL_SOURCE_SAWTOOTH_RIGHT_WAVE_H__







