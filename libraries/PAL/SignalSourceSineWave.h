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
    
    static const uint16_t DEFAULT_PHASE_OFFSET = 0;
    static const uint16_t DEFAULT_PHASE_STEP   = 1;
    
    constexpr static const double SCALING_RATIO = (double)SAMPLE_COUNT / 360.0;
    
public:
    SignalSourceSineWave()
    {
        Reset(DEFAULT_PHASE_STEP, DEFAULT_PHASE_OFFSET);
    }
    
    inline void Reset(uint16_t phaseStep, uint16_t phaseOffset = 0)
    {
        // Set up so that when GetNextSampleReady is complete, the value of
        // idxCurrent is equal to the phaseOffset specified and the sample
        // there is ready to be read.
        idxStep_    = phaseStep * SCALING_RATIO;
        idxCurrent_ = (phaseOffset * SCALING_RATIO);
        idxCurrent_ = (idxCurrent_ - idxStep_) % SAMPLE_COUNT;
        
        // Acquire sample
        GetNextSampleReady();
    }
    
    inline void ChangePhaseStep(uint16_t phaseStep)
    {
        // Rewind a step
        idxCurrent_ = (idxCurrent_ - idxStep_) % SAMPLE_COUNT;
        
        // Prepare new step size
        idxStep_ = phaseStep * SCALING_RATIO;
        
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













