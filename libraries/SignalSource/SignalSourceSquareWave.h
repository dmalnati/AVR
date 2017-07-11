#ifndef __SIGNAL_SOURCE_SQUARE_WAVE_H__
#define __SIGNAL_SOURCE_SQUARE_WAVE_H__


#include "SignalSource.h"


class SignalSourceSquareWave
: public SignalSource
{
public:

    inline virtual int8_t GetSample(const uint8_t brad) override
    {
        int8_t val = (brad < 128 ? -127 : 127);
        
        return val;
    }

};


#endif  // __SIGNAL_SOURCE_SQUARE_WAVE_H__




