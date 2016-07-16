#ifndef __SIGNAL_SOURCE_SINE_WAVE_H__
#define __SIGNAL_SOURCE_SINE_WAVE_H__


#include <avr/pgmspace.h>

#include <math.h>


extern const uint8_t SINE_TABLE[] PROGMEM;


// Effectively a singleton, but you need to instantiate an instance to use it
// so that its static vars get set up correctly.
//
// Usage:
// - Reset()
// - GetSample(), GetNextSampleReady(), ...
// - ChangePhaseStep(), GetSample(), GetNextSampleReady(), ...
//
// Doing Reset(), ChangePhaseStep() won't work!

class SignalSourceSineWave
{
    static const uint16_t SAMPLE_COUNT = 512;
    
    constexpr static const double DEFAULT_PHASE_PCT = 1.0;
    
public:
    SignalSourceSineWave()
    {
        PhaseConfig cfg;
        GetPhaseConfig(DEFAULT_PHASE_PCT, &cfg);
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
        uint16_t idxStep;
        
        // Useful for debugging
        double phaseRequested;
    };
    
    static uint8_t GetPhaseConfig(double phasePct, PhaseConfig *cfg)
    {
        uint8_t retVal = 1;
        
        cfg->idxStep        = phasePct / 100.0 * SAMPLE_COUNT;
        cfg->phaseRequested = phasePct;
        
        return retVal;
    }
    
    static inline void Reset(PhaseConfig *cfg)
    {
        // Set up so that when GetNextSampleReady is complete, the value of
        // idxCurrent is equal to the phaseOffset specified and the sample
        // there is ready to be read.
        idxStep_    = cfg->idxStep;
        idxCurrent_ = (idxCurrent_ - idxStep_) % SAMPLE_COUNT;
        
        // Acquire sample
        GetNextSampleReady();
    }
    
    static inline void ChangePhaseStep(PhaseConfig *cfg)
    {
        // Rewind a step
        idxCurrent_ = (idxCurrent_ - idxStep_) % SAMPLE_COUNT;
        
        // Prepare new step size
        idxStep_ = cfg->idxStep;
        
        // Acquire sample
        GetNextSampleReady();
    }
    
    static inline uint8_t GetSample()
    {
        return sample_;
    }
    
    static inline void GetNextSampleReady()
    {
        idxCurrent_ = (idxCurrent_ + idxStep_) % SAMPLE_COUNT;
        
        uint16_t pgmByteLocation = (uint16_t)SINE_TABLE + idxCurrent_;
        
        sample_ = pgm_read_byte_near(pgmByteLocation);
    }
    
private:

    static uint8_t sample_;
    
    static uint16_t idxCurrent_;
    static uint16_t idxStep_;
};


#endif  // __SIGNAL_SOURCE_SINE_WAVE_H__













