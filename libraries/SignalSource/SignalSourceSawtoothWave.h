#ifndef __SIGNAL_SOURCE_SAWTOOTH_WAVE_H__
#define __SIGNAL_SOURCE_SAWTOOTH_WAVE_H__


#include "FixedPoint.h"


class SignalSourceSawtoothWave
{
    static const uint16_t UPPER_LIMIT = 255;

    class Stepper
    {
        friend class SignalSourceSawtoothWave;
        
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
            return (uint8_t)val_;
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


#endif  // __SIGNAL_SOURCE_SAWTOOTH_WAVE_H__







