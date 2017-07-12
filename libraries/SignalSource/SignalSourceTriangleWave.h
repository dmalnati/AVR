#ifndef __SIGNAL_SOURCE_TRIANGLE_WAVE_H__
#define __SIGNAL_SOURCE_TRIANGLE_WAVE_H__


class SignalSourceTriangleWave
{
public:

    static inline int8_t GetSample(const uint8_t brad)
    {
        int8_t val;
        
        if (brad < 128)
        {
            val = (brad << 1) + 1;
        }
        else if (brad == 128)
        {
            val = 255;
        }
        else if (brad < 255)
        {
            val = (255 - (((brad - 128) + 1) << 1));
        }
        else if (brad == 255)
        {
            val = 0;
        }
        
        val -= 128;
        
        return val;
    }

};


#endif  // __SIGNAL_SOURCE_TRIANGLE_WAVE_H__




