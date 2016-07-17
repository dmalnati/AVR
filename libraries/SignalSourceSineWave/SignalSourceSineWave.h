#ifndef __SIGNAL_SOURCE_SINE_WAVE_H__
#define __SIGNAL_SOURCE_SINE_WAVE_H__


#include <avr/pgmspace.h>

#include <math.h>

// Debug
#include "Arduino.h"


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
    
    // Rounding error values
    static const uint8_t  DECIMAL_PLACES           = 4;
    static const uint16_t ERR_PCT_SCALING_FACTOR   = pow(10, DECIMAL_PLACES);
    static const uint16_t ROUNDING_ERROR_THRESHOLD = ERR_PCT_SCALING_FACTOR;
    
public:
    SignalSourceSineWave()
    {
        // Nothing to do
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
        
        // Deal with rounding errors
        uint16_t roundingError;
        int8_t   roundingErrorAdjustment;
        
        // Useful for debugging
        double phaseRequested;
    };
    
    static uint8_t GetPhaseConfig(double phasePct, PhaseConfig *cfg)
    {
        uint8_t retVal = 1;
        
        cfg->idxStep        = round(phasePct / 100.0 * SAMPLE_COUNT);
        cfg->phaseRequested = phasePct;
        
        // Calculate values used in adjusting for rounding errors
        double pctOfTableSize = (double)cfg->idxStep / SAMPLE_COUNT * 100.0;
        double pctDiff        = (pctOfTableSize - phasePct) / phasePct;

        double idxStepLogical = phasePct / 100.0 * SAMPLE_COUNT;
        double stepDiff       = idxStepLogical * pctDiff;
        
        cfg->roundingError = fabs(stepDiff) * ERR_PCT_SCALING_FACTOR;
        if (stepDiff < 0)
        {
            cfg->roundingErrorAdjustment = 1;
        }
        else
        {
            cfg->roundingErrorAdjustment = -1;
        }

        Serial.print("idxStepLogical:");
        Serial.println(idxStepLogical);
        Serial.print("stepDiff:");
        Serial.println(stepDiff);
        Serial.print("roundingError:");
        Serial.println(cfg->roundingError);
        Serial.print("roundingErrorAdjustment:");
        Serial.println(cfg->roundingErrorAdjustment);
        
        
        
        return retVal;
    }
    
    static inline void Reset(PhaseConfig *cfg)
    {
        // Set up so that when GetNextSampleReady is complete, the value of
        // idxCurrent is equal to the phaseOffset specified and the sample
        // there is ready to be read.
        idxStep_    = cfg->idxStep;
        idxCurrent_ = (idxCurrent_ - idxStep_) % SAMPLE_COUNT;
        
        // Adjust rounding error state
        roundingErrorCumulative_ = 0;
        
        roundingError_           = cfg->roundingError;
        roundingErrorAdjustment_ = cfg->roundingErrorAdjustment;
        
        // Acquire sample
        GetNextSampleReady();
    }
    
    static inline void ChangePhaseStep(PhaseConfig *cfg)
    {
        // Rewind a step
        idxCurrent_ = (idxCurrent_ - idxStep_) % SAMPLE_COUNT;

        // Prepare new step size
        idxStep_ = cfg->idxStep;
        
        // Accept new rounding error handling vars.  Keep current cumulative
        // quantity of error set as-is, though.  (unsure)
        //roundingErrorCumulative_ = 0;
        
        roundingError_           = cfg->roundingError;
        roundingErrorAdjustment_ = cfg->roundingErrorAdjustment;
        
        // Acquire sample
        GetNextSampleReady();
    }
    
    static inline uint8_t GetSample()
    {
        return sample_;
    }
    
    static inline void GetNextSampleReady()
    {
        // Deal with rounding error
        int8_t adjustment;
        
        roundingErrorCumulative_ += roundingError_;
        if (roundingErrorCumulative_ >= ROUNDING_ERROR_THRESHOLD)
        {
            adjustment = roundingErrorAdjustment_;

            roundingErrorCumulative_ -= ROUNDING_ERROR_THRESHOLD;
        }
        else
        {
            adjustment = 0;
        }
        
        idxCurrent_ = (idxCurrent_ + idxStep_ + adjustment) % SAMPLE_COUNT;
        
        uint16_t pgmByteLocation = (uint16_t)SINE_TABLE + idxCurrent_;
        
        sample_ = pgm_read_byte_near(pgmByteLocation);
    }
    
private:

    static uint8_t sample_;
    
    static uint16_t idxCurrent_;
    static uint16_t idxStep_;
    
    // Rounding error
    static uint16_t roundingErrorCumulative_;
    
    static uint16_t roundingError_;
    static int8_t   roundingErrorAdjustment_;
};


#endif  // __SIGNAL_SOURCE_SINE_WAVE_H__













