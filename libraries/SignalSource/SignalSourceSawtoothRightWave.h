#ifndef __SIGNAL_SOURCE_SAWTOOTH_RIGHT_WAVE_H__
#define __SIGNAL_SOURCE_SAWTOOTH_RIGHT_WAVE_H__


class SignalSourceSawtoothRightWave
{
public:
    
    static inline int8_t GetSample(const uint8_t brad)
    {
        int8_t val = brad - 128;
        
        return val;
    }

};


#endif  // __SIGNAL_SOURCE_SAWTOOTH_RIGHT_WAVE_H__







