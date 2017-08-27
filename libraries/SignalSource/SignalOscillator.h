#ifndef __SIGNAL_OSCILLATOR_H__
#define __SIGNAL_OSCILLATOR_H__


#include "FixedPoint.h"
#include "FixedPointStepper.h"


struct SignalOscillatorFrequencyConfig
{
    uint16_t frequency = 1;
    Q88      stepSize;
};


class SignalOscillator
{
    static const uint16_t VALUE_COUNT = 256;
    
    using SignalSourceFn = int8_t(*)(uint8_t);

    
public:
    
    SignalOscillator(SignalSourceFn ssFn)
    : ssFn_(ssFn)
    {
        // Nothing to do
    }

    void SetSampleRate(uint16_t sampleRate)
    {
        stepSizePerHz_ = (double)VALUE_COUNT / (double)sampleRate;
    }
    
    SignalOscillatorFrequencyConfig GetFrequencyConfig(uint16_t frequency)
    {
        SignalOscillatorFrequencyConfig fc;
        
        fc.frequency = frequency;
        
        // Use full-resolution double for an infrequently-called function
        // to calculate step size for stepper
        fc.stepSize = stepSizePerHz_ * fc.frequency;
        
        return fc;
    }
    
    void SetFrequencyByConfig(SignalOscillatorFrequencyConfig fc)
    {
        frequency_ = fc.frequency;
        rotation_.SetStepSize(fc.stepSize);
    }
    
    void SetFrequency(uint16_t frequency)
    {
        frequency_ = frequency;
        
        // Use full-resolution double for an infrequently-called function
        // to calculate step size for stepper
        stepSize_ = stepSizePerHz_ * frequency;
        
        rotation_.SetStepSize(stepSize_);
    }
    
    uint16_t GetFrequency()
    {
        return frequency_;
    }

    void ApplyFrequencyOffsetPctIncreaseFromBase(Q08 pctIncrease)
    {
        Q88 stepSizeToAdd = stepSize_ * pctIncrease;
        
        Q88 stepSizeNew = stepSize_ + stepSizeToAdd;
        
        rotation_.SetStepSize(stepSizeNew);
    }
    
    void SetPhaseOffset(int8_t phaseOffset)
    {
        phaseOffset_ = phaseOffset;
    }
    
    void SetSignalSource(SignalSourceFn ssFn)
    {
        ssFn_ = ssFn;
    }
    
    inline int8_t GetNextSample()
    {
        // Keep the ordering of these instructions.  Bringing the Incr ahead of
        // GetSample costs ~500ns for some reason.
        uint8_t brad = phaseOffset_ + (uint8_t)rotation_;
        
        int8_t retVal = ssFn_(brad);

        rotation_.Incr();
        
        return retVal;
    }
    
    void Reset()
    {
        rotation_.SetValue((Q88)(uint8_t)0);
    }

private:



    double                  stepSizePerHz_;
    Q88                     stepSize_;
    SignalSourceFn          ssFn_;
    FixedPointStepper<Q88>  rotation_;
    int8_t                  phaseOffset_ = 0;
    uint16_t                frequency_ = 0;
};


#endif  // __SIGNAL_OSCILLATOR_H__
















