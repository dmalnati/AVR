#ifndef __SIGNAL_OSCILLATOR_H__
#define __SIGNAL_OSCILLATOR_H__


#include "FixedPoint.h"
#include "FixedPointStepper.h"


class SignalOscillator
{
    static const uint16_t VALUE_COUNT = 256;
    
    using SignalSourceFn = int8_t(*)(uint8_t);

    
public:
    void SetSampleRate(uint16_t sampleRate)
    {
        stepSizePerHz_ = (double)VALUE_COUNT / (double)sampleRate;
        
        stepSizePerHzQ_ = stepSizePerHz_;
    }
    
    void SetFrequency(uint16_t frequency)
    {
        // Use full-resolution double for an infrequently-called function
        stepSize_ = stepSizePerHz_ * frequency;
        
        rotation_.SetStepSize(stepSize_);
    }
    
    void ApplyFrequencyOffset(int8_t frequencyOffset)
    {
        if (frequencyOffset >= 0)
        {
            // Convert frequency offset to fast double
            Q88 frequencyOffsetQ = (uint8_t)frequencyOffset;
            
            // Scale the frequency offset by the steps-per-hz multiplier
            Q88 stepOffset = stepSizePerHzQ_ * frequencyOffsetQ;
            
            // Apply offset to cached step size
            Q88 stepSizeNew = stepSize_ + stepOffset;
            
            // Set new step size
            rotation_.SetStepSize(stepSizeNew);
        }
        else
        {
            // Convert frequency offset to fast double
            Q88 frequencyOffsetQ = (uint8_t)-frequencyOffset;

            // Scale the frequency offset by the steps-per-hz multiplier
            Q88 stepOffset = stepSizePerHzQ_ * frequencyOffsetQ;
            
            // Apply offset to cached step size
            Q88 stepSizeNew = stepSize_ - stepOffset;

            // Set new step size
            rotation_.SetStepSize(stepSizeNew);
        }
    }
    
    void SetSignalSource(SignalSourceFn ssFn)
    {
        ssFn_ = ssFn;
    }
    
    inline int8_t GetNextSample()
    {
        // Keep the ordering of these instructions.  Bringing the Incr ahead of
        // GetSample costs ~500ns for some reason.
        uint8_t brad = rotation_.GetUnsignedInt8();
        
//        int8_t retVal = (*ss_)(brad);
        int8_t retVal = ssFn_(brad);

        rotation_.Incr();
        
        return retVal;
    }
    
    void Reset()
    {
        rotation_.SetValue(0);
    }

private:



    double                  stepSizePerHz_;
    Q88                     stepSizePerHzQ_;
    SignalSourceFn          ssFn_;
    FixedPointStepper<Q88>  rotation_;
    Q88                     stepSize_;
};


#endif  // __SIGNAL_OSCILLATOR_H__
















