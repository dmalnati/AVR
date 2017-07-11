#ifndef __SIGNAL_OSCILLATOR_H__
#define __SIGNAL_OSCILLATOR_H__


#include "FixedPoint.h"
#include "FixedPointStepper.h"


template <typename SignalSource>
class SignalOscillator
{
    static const uint16_t VALUE_COUNT = 256;
    
public:
    void SetSampleRate(uint16_t sampleRate)
    {
        sampleRate_ = sampleRate;
    }
    
    void SetFrequency(uint16_t frequency)
    {
        double stepSize = 
                (double)VALUE_COUNT / ((double)sampleRate_ / (double)frequency);
        
        rotation_.SetStepSize(stepSize);
    }
    
    inline int8_t GetNextSample()
    {
        // Keep the ordering of these instructions.  Bringing the Incr ahead of
        // GetSample costs ~500ns for some reason.
        uint8_t brad = rotation_.GetUnsignedInt8();
        
        int8_t retVal = ss_.GetSample(brad);

        rotation_.Incr();
        
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
















