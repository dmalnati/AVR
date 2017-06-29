#ifndef __SIGNAL_SOURCE_TRIANGLE_WAVE_H__
#define __SIGNAL_SOURCE_TRIANGLE_WAVE_H__


#include "FixedPoint.h"


class SignalSourceTriangleWave
{
    static const uint16_t UPPER_LIMIT = 512;
    static const uint16_t HALF_LIMIT  = UPPER_LIMIT / 2;

    class Stepper
    {
        friend class SignalSourceTriangleWave;
        
    public:
        
        void Calibrate(uint16_t sampleRate, uint16_t frequency)
        {
            if (sampleRate && frequency)
            {
                stepSize_ =
                    (double)UPPER_LIMIT / ((double)sampleRate / (double)frequency);
            }
        }
    
        inline void operator++()
        {
            val_ += stepSize_;
            
            if (val_ > UPPER_LIMIT)
            {
                val_ -= UPPER_LIMIT;
            }
        }
        
        void Reset()
        {
            val_ = 0.0;
        }
        
    private:
    
        inline operator uint8_t() const
        {
            uint16_t val = (uint16_t)val_;
            
            uint8_t retVal = 0;
            
            if (val < HALF_LIMIT)
            {
                retVal = (uint8_t)val;
            }
            else
            {
                retVal = (uint8_t)(UPPER_LIMIT - val - 1);
            }
            
            return retVal;
        }
    
        Q1616 val_      { 0.0 };
        Q1616 stepSize_ { 0.0 };
    };

public:

    using IdxType = Stepper;
    
    static inline uint8_t GetSampleAtIdx(const IdxType &idx)
    {
        uint8_t val = (uint8_t)idx;
        
        return val;
    }

private:
};

#endif  // __SIGNAL_SOURCE_TRIANGLE_WAVE_H__




