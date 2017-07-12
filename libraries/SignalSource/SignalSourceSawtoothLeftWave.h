#ifndef __SIGNAL_SOURCE_SAWTOOTH_LEFT_WAVE_H__
#define __SIGNAL_SOURCE_SAWTOOTH_LEFT_WAVE_H__


class SignalSourceSawtoothLeftWave
{
public:
    
    static inline int8_t GetSample(const uint8_t brad)
    {
        int8_t val = 127 - brad;
        
        return val;
    }

};


#endif  // __SIGNAL_SOURCE_SAWTOOTH_LEFT_WAVE_H__







