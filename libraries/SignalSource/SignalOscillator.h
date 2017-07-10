#ifndef __SIGNAL_OSCILLATOR_H__
#define __SIGNAL_OSCILLATOR_H__


#include "FixedPoint.h"
#include "FixedPointStepper.h"


template <typename SignalSource>
class SignalOscillator
{
    static const uint16_t TOP_VALUE = 256;
    
public:
    void SetSampleRate(uint16_t sampleRate)
    {
        sampleRate_ = sampleRate;
    }
    
    void SetFrequency(uint16_t frequency)
    {
        double stepSize = 
                (double)TOP_VALUE / ((double)sampleRate_ / (double)frequency);
        
        rotation_.SetStepSize(stepSize);
    }
    
    int8_t GetNextSample()
    {
        uint8_t brad  = (uint8_t)rotation_;
        int8_t retVal = ss_.GetSample(brad);
        
        ++rotation_;
        
        return retVal;
    }
    
    void Reset()
    {
        rotation_.SetValue(0);
    }

private:

    uint16_t               sampleRate_;
    SignalSource           ss_;
    FixedPointStepper<Q88> rotation_;
};


#endif  // __SIGNAL_OSCILLATOR_H__
















