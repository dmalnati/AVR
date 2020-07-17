#ifndef __SIGNAL_OSCILLATOR_H__
#define __SIGNAL_OSCILLATOR_H__


#include "FixedPoint.h"
#include "FixedPointStepper.h"
#include "SignalSourceNoneWave.h"


struct SignalOscillatorFrequencyConfig
{
    double   frequency = 1;
    Q88      stepSize;
};



/*
 * Class designed to allow for the dynamic creation of a continuous signal.
 * Class implementation is designed for maximum speed.
 *   Uses integer-based approximations of non-integer values, so some error.
 * 
 * Main points:
 * - Calling code determines the signal source to use
 *   - eg sine wave, triangle, etc
 *   - each represents a repeating signal
 *     - not bounded to a specific timeframe
 *     - the intent is to stretch/squeeze this signal into whatever desired
 *       frequency
 *       - eg, produce a sine wave at 440Hz, where I sample 10,000 times/sec
 * 
 * - Calling code determines the sample rate (times/sec) it will ask for new
 *   samples from this class
 * 
 * - Calling code determines the frequency of the selected signal source
 * 
 * - Signal values are supported within a 256 value range.  Two modes:
 *   - Centered on 0 -- meaning both positive and negative: -128 to 127
 *   - Absolute      -- meaning positive values only:          0 to 255
 * 
 * Multiple SignalOscillators can be used in parallel.
 * - eg, multi instance, each configured for same sample rate
 * 
 * Binary Radians - brads
 * - steps around the signal are in a range of 0-255 (not degrees 0-360)
 * 
 * In multi-mode, phase shifts between can be configured for each.  eg:
 * - o1, 10kHz sine at 440Hz
 * - o2, 10kHz sine at 440Hz
 *   - o2 has *180 degree phase shift
 *     - notably, 180 degree must be converted to brad scale of 0-255
 * - provided each are sampled together, they'll stay in sync forever
 * 
 * 
 * Example use:
 * ------------
 * SignalOscillator o(SignalSourceSineWave::GetSample)
 * o.SetSampleRate(10000);  
 * o.SetFrequency(440);
 * while (1)
 * {
 *     uint8_t val = o.GetNextSampleAbs();
 *     // do something with it
 *     // sleep until next 100us marker
 * }
 * 
 * 
 * Other Notes
 * - Class original design was for audio synthesis.
 *   - So expected hundreds-to-thousands of Hz for signals
 * - Use cases where signals are meant to be multi-second need to consider
 *   implementation details
 *   - eg, stretching a sine wave over 5 seconds
 *   - the sine wave is 256 discrete points behind the scenes
 *   - sampling at 10,000 Hz is pointless
 *   - the most frequently you can get a different sample is period-in-secs/256
 *     - eg 1 second frequency = 1/256 = every  4 ms
 *     - eg 5 second frequency = 5/256 = every 19 ms
 *     - not bad, but worth avoiding extreme sampling in these cases
 * 
 * For maximum future compatibility, ignore implementation details.
 * Set whatever sampling frequency makes sense.
 * 
 * 
 */


class SignalOscillator
{
    static const uint16_t VALUE_COUNT = 256;
    
    using SignalSourceFn = int8_t(*)(uint8_t);

    
public:
    
    SignalOscillator(SignalSourceFn ssFn = SignalSourceNoneWave::GetSample)
    {
        SetSignalSource(ssFn);
    }

    void SetSampleRate(uint16_t sampleRate)
    {
        stepSizePerHz_ = (double)VALUE_COUNT / (double)sampleRate;
    }
    
    SignalOscillatorFrequencyConfig GetFrequencyConfig(double frequency)
    {
        SignalOscillatorFrequencyConfig fc;
        
        fc.frequency = frequency;
        
        // Use full-resolution double for an infrequently-called function
        // to calculate step size for stepper
        fc.stepSize = stepSizePerHz_ * fc.frequency;
        
        return fc;
    }
    
    void SetFrequencyByConfig(SignalOscillatorFrequencyConfig *fc)
    {
        frequency_ = fc->frequency;
        rotation_.SetStepSize(fc->stepSize);
    }
    
    void SetFrequency(double frequency)
    {
        frequency_ = frequency;
        
        // Use full-resolution double for an infrequently-called function
        // to calculate step size for stepper
        stepSize_ = stepSizePerHz_ * frequency;
        
        rotation_.SetStepSize(stepSize_);
    }
    
    double GetFrequency()
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

    inline uint8_t GetNextSampleAbs()
    {
        return 128 + GetNextSample();
    }

    Q88::INTERNAL_STORAGE_TYPE GetRotationState() const
    {
        return rotation_.GetValueState();
    }

    void ReplaceRotationState(Q88::INTERNAL_STORAGE_TYPE rotation)
    {
        rotation_.ReplaceValueState(rotation);
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
    double                  frequency_ = 0;
};


#endif  // __SIGNAL_OSCILLATOR_H__
















