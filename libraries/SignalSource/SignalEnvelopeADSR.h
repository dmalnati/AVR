#ifndef __SIGNAL_ENVELOPE_ADSR__
#define __SIGNAL_ENVELOPE_ADSR__


#include "FixedPoint.h"


class SignalEnvelopeADSR
{
    static const uint8_t BOTTOM_VALUE =   0;
    static const uint8_t TOP_VALUE    = 255;
    
    static const uint16_t DEFAULT_SAMPLE_RATE = 1;
    static const uint16_t DEFAULT_STEP_SIZE   = 0;
    
    static const uint16_t DEFAULT_ATTACK_DURATION_MS  = 50;
    static const uint16_t DEFAULT_DECAY_DURATION_MS   = 100;
    static const uint8_t  DEFAULT_SUSTAIN_LEVEL_PCT   = 50;
    static const uint16_t DEFAULT_RELEASE_DURATION_MS = 20;
    
public:
    
    void SetSampleRate(uint16_t sampleRate)
    {
        sampleRate_ = sampleRate;
        
        Recalculate();
    }

    void SetAttackDuration(uint16_t durationMs)
    {
        attackDurationMs_ = durationMs;
        
        Recalculate();
    }
    
    void SetDecayDuration(uint16_t durationMs)
    {
        decayDurationMs_ = durationMs;
        
        Recalculate();
    }
    
    void SetSustainLevelPct(uint8_t pct)
    {
        sustainLevelPct_ = pct;
        
        Recalculate();
    }
    
    void SetReleaseDuration(uint16_t durationMs)
    {
        releaseDurationMs_ = durationMs;
        
        Recalculate();
    }

    
    void StartAttack()
    {
        Reset();
    }
    
    void StartDecay()
    {
        state_ = State::RELEASE;
    }
    
    // return a scaling factor
    Q08 GetNextEnvelope()
    {
        uint8_t retVal = 0;
        
        if (state_ == State::ATTACK)
        {
            // Convert to decimal value
            retVal = attackStepper_.GetUnsignedInt8();
            
            // Check if time to move to decay for next sample
            if (retVal == TOP_VALUE)
            {
                state_ = State::DECAY;
            }
            else
            {
                attackStepper_.IncrTowardLimit();
            }
        }
        else if (state_ == State::DECAY)
        {
            // Convert to decimal value
            retVal = decayStepper_.GetUnsignedInt8();
            
            // Check if time to move to sustain for next sample
            if (retVal == sustainLevel_)
            {
                state_ = State::SUSTAIN;
            }
            else
            {
                decayStepper_.DecrTowardLimit();
            }
        }
        else if (state_ == State::SUSTAIN)
        {
            // Constant value.
            // No transition to another state from here, that happens when the
            // caller invokes StartDecay()
            retVal = sustainLevel_;
        }
        else if (state_ == State::RELEASE)
        {
            // Convert to decimal value
            retVal = releaseStepper_.GetUnsignedInt8();
            
            // Check if time to move to final state
            if (retVal == BOTTOM_VALUE)
            {
                state_ = State::DONE;
            }
            else
            {
                releaseStepper_.DecrTowardLimit();
            }
        }
        
        return (Q08)retVal;
    }
    
    void Reset()
    {
        state_ = State::ATTACK;
        
        SetStepwiseInitialValues();
    }
    

private:

    void Recalculate()
    {
        CalculateAttack();
        CalculateSustainLevel();
        CalculateDecay();
        CalculateRelease();
        
        SetStepwiseInitialValues();
    }
    
    double CalculateLinearStepSize(uint16_t deltaY, uint16_t durationMs)
    {
        double stepSize =
            (double)(deltaY) / 
            (((double)sampleRate_ / 1000.0) * (double)durationMs);
            
        return stepSize;
    }

    void CalculateAttack()
    {
        // Just go linear up
        double stepSize = CalculateLinearStepSize(TOP_VALUE - BOTTOM_VALUE,
                                                  attackDurationMs_);
        
        // Store calculated value
        attackStepper_.SetStepSize(stepSize);
    }
    
    void CalculateSustainLevel()
    {
        sustainLevel_ = (double)sustainLevelPct_ / 100.0 * TOP_VALUE;
    }
    
    void CalculateDecay()
    {
        // Just go linear down
        double stepSize = CalculateLinearStepSize(TOP_VALUE - sustainLevel_,
                                                  decayDurationMs_);
        
        // Store calculated value
        decayStepper_.SetStepSize(stepSize);
    }
    
    void CalculateRelease()
    {
        // Just go linear down
        double stepSize = CalculateLinearStepSize(sustainLevel_ - BOTTOM_VALUE,
                                                  releaseDurationMs_);
        
        // Store calculated value
        releaseStepper_.SetStepSize(stepSize);
    }
    
    void SetStepwiseInitialValues()
    {
        attackStepper_.SetValue((double)BOTTOM_VALUE);
        attackStepper_.SetLimitUpper((double)TOP_VALUE);
        
        decayStepper_.SetValue((double)TOP_VALUE);
        decayStepper_.SetLimitLower((double)sustainLevel_);
        
        releaseStepper_.SetValue((double)sustainLevel_);
        releaseStepper_.SetLimitLower((double)BOTTOM_VALUE);
    }
    
    
    enum class State : uint8_t
    {
        ATTACK = 0,
        DECAY,
        SUSTAIN,
        RELEASE,
        DONE
    };

    // Keep track of state of envelope stage
    State state_;
    
    // Cached raw values
    uint16_t sampleRate_ = DEFAULT_SAMPLE_RATE;
    
    // Attack values
    uint16_t attackDurationMs_ = DEFAULT_ATTACK_DURATION_MS;
    FixedPointStepper<Q88>  attackStepper_;
    
    // Decay values
    uint16_t decayDurationMs_ = DEFAULT_DECAY_DURATION_MS;
    FixedPointStepper<Q88>  decayStepper_;

    // Sustain values
    uint8_t sustainLevelPct_ = DEFAULT_SUSTAIN_LEVEL_PCT;
    uint8_t sustainLevel_    = ((double)DEFAULT_SUSTAIN_LEVEL_PCT / 100.0) *
                               TOP_VALUE;
    
    // Release values
    uint16_t releaseDurationMs_ = DEFAULT_RELEASE_DURATION_MS;
    FixedPointStepper<Q88>  releaseStepper_;
};


#endif  // __SIGNAL_ENVELOPE_ADSR__






