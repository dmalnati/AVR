#ifndef __SIGNAL_SOURCE_NONE_WAVE_H__
#define __SIGNAL_SOURCE_NONE_WAVE_H__


class SignalSourceNoneWave
{
    static const int8_t DEFAULT_VALUE = 0;
public:

    static inline int8_t GetSample(const uint8_t)
    {
        return DEFAULT_VALUE;
    }

};


#endif  // __SIGNAL_SOURCE_NONE_WAVE_H__




