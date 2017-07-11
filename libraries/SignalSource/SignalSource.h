#ifndef __SIGNAL_SOURCE_H__
#define __SIGNAL_SOURCE_H__


class SignalSource
{
    static const int8_t DEFAULT_VALUE = -128;
    
public:
    virtual int8_t GetSample(const uint8_t /* brad */)
    {
        return DEFAULT_VALUE;
    }
};


#endif  // __SIGNAL_SOURCE_H__