#ifndef __SIGNAL_SOURCE_SINE_WAVE_H__
#define __SIGNAL_SOURCE_SINE_WAVE_H__


#include <avr/pgmspace.h>

#include "FixedPoint.h"


extern const uint8_t SINE_TABLE[] PROGMEM;


class SignalSourceSineWave
{
    static const uint16_t SINE_TABLE_LEN = 512;
    
    
    class StepperTypeFixedPointDouble
    {
        friend class SignalSourceSineWave;
        
        using ThisClass = StepperTypeFixedPointDouble;
        
    public:
        StepperTypeFixedPointDouble()
        {
            // Nothing to do
        }
        
        StepperTypeFixedPointDouble(uint16_t sampleRate, uint16_t frequency)
        {
            Calibrate(sampleRate, frequency);
        }
        
        void Calibrate(uint16_t sampleRate, uint16_t frequency)
        {
            // only change stepSize_, leave idx_ so that this (re)calibration
            // can be run whenever.
            
            if (sampleRate && frequency)
            {
                stepSize_ =
                    (double)SINE_TABLE_LEN / ((double)sampleRate / (double)frequency);
            }
        }
    
        inline void operator++()
        {
            idx_ += stepSize_;
            
            if (idx_ > SINE_TABLE_LEN)
            {
                idx_ -= SINE_TABLE_LEN;
            }
        }
        
        void ResetIdx()
        {
            idx_ = 0.0;
        }
        
    private:
    
        inline operator uint16_t() const
        {
            return (uint16_t)idx_;
        }
        
        Q1616 idx_      { 0.0 };
        Q1616 stepSize_ { 0.0 };
    };
    
    
public:
    SignalSourceSineWave()
    {
        // Nothing to do
    }
    
    using IdxType = StepperTypeFixedPointDouble;
    
    static inline uint8_t GetSampleAtIdx(const IdxType &idx)
    {
        uint16_t sineTableIdx    = (uint16_t)idx;
        uint16_t pgmByteLocation = (uint16_t)SINE_TABLE + sineTableIdx;
        uint8_t val              = pgm_read_byte_near(pgmByteLocation);
        
        return val;
    }
    
};


#endif  // __SIGNAL_SOURCE_SINE_WAVE_H__













