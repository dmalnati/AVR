#ifndef __SIGNAL_SOURCE_SINE_WAVE_H__
#define __SIGNAL_SOURCE_SINE_WAVE_H__


#include <avr/pgmspace.h>


extern const uint8_t SINE_TABLE[] PROGMEM;


// Usage:
// - Reset()
// - GetSample(), GetNextSampleReady(), ...
// - ChangePhaseStep(), GetSample(), GetNextSampleReady(), ...
//
// Doing Reset(), ChangePhaseStep() won't work!

class SignalSourceSineWave
{
    static const uint16_t SAMPLE_COUNT = 512;
    
    static const uint16_t DEFAULT_PHASE_STEP   = 1;
    
    constexpr static const double SCALING_RATIO = (double)SAMPLE_COUNT / 360.0;
    
public:
    SignalSourceSineWave()
    {
        PhaseConfig cfg;
        GetPhaseConfig(DEFAULT_PHASE_STEP, &cfg);
        Reset(&cfg);
    }
    
    // Allow for pre-calculation of phase-related values.
    // Avoiding the multiplication steps at run-time saves ~30us.
    //
    // None of the modulus calculations can be, or need to be, done in advance
    // since the compiler has been measured to be optimizing the power-of-two
    // situation anyway.
    struct PhaseConfig
    {
        // Actually useful configuration
        uint16_t phasePreScaled;
        
        // Useful for debugging
        uint16_t phaseRequested;
    };
    
    uint8_t GetPhaseConfig(uint16_t phase, PhaseConfig *cfg)
    {
        uint8_t retVal = 1;
        
        cfg->phasePreScaled = phase * SCALING_RATIO;
        cfg->phaseRequested = phase;
        
        return retVal;
    }
    
    inline void Reset(PhaseConfig *cfgPhaseStep)
    {
        // Set up so that when GetNextSampleReady is complete, the value of
        // idxCurrent is equal to the phaseOffset specified and the sample
        // there is ready to be read.
        idxStep_    = cfgPhaseStep->phasePreScaled;
        idxCurrent_ = (idxCurrent_ - idxStep_) % SAMPLE_COUNT;
        
        // Acquire sample
        GetNextSampleReady();
    }
    
    inline void ChangePhaseStep(PhaseConfig *cfgPhaseStep)
    {
        // Rewind a step
        idxCurrent_ = (idxCurrent_ - idxStep_) % SAMPLE_COUNT;
        
        // Prepare new step size
        idxStep_ = cfgPhaseStep->phasePreScaled;
        
        // Acquire sample
        GetNextSampleReady();
    }
    
    inline uint8_t GetSample()
    {
        return sample_;
    }
    
    inline void GetNextSampleReady()
    {
        idxCurrent_ = (idxCurrent_ + idxStep_) % SAMPLE_COUNT;
        
        uint16_t pgmByteLocation = (uint16_t)SINE_TABLE + idxCurrent_;
        
        sample_ = pgm_read_byte_near(pgmByteLocation);
    }
    
private:

    uint8_t sample_;
    
    uint16_t idxCurrent_;
    uint16_t idxStep_;
};


#endif  // __SIGNAL_SOURCE_SINE_WAVE_H__













