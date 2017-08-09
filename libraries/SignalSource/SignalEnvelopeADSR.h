#ifndef __SIGNAL_ENVELOPE_ADSR__
#define __SIGNAL_ENVELOPE_ADSR__


#include "FixedPoint.h"


class SignalEnvelopeADSR
{
    static const uint8_t BOTTOM_VALUE =   0;
    static const uint8_t TOP_VALUE    = 255;
    
    static const uint16_t DEFAULT_SAMPLE_RATE = 1;
    static const uint16_t DEFAULT_STEP_SIZE   = 0;
    
    static const uint16_t DEFAULT_ATTACK_DURATION_MS  = 100;
    static const uint16_t DEFAULT_DECAY_DURATION_MS   = 100;
    static const uint8_t  DEFAULT_SUSTAIN_LEVEL_PCT   = 80;
    static const uint16_t DEFAULT_RELEASE_DURATION_MS = 350;
    
public:

    SignalEnvelopeADSR()
    {
        SetConstantValues();

        Reset();
    }
    
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
        
        state_ = State::ATTACK;
        // Serial.println("Attack");
        // Serial.print("attackDurationMs_ "); Serial.print(attackDurationMs_);
        // Serial.print("decayDurationMs_ "); Serial.print(decayDurationMs_);
        // Serial.print("sustainLevelPct_ "); Serial.print(sustainLevelPct_);
        // Serial.print("sustainLevel_ "); Serial.print(sustainLevel_);
        // Serial.print("releaseDurationMs_ "); Serial.print(releaseDurationMs_);
        // Serial.println();
    }
    
    void StartRelease()
    {
        // Calculate starting point for Decay.
        // Can't pre-calculate because Attack or Decay may still be in effect.
        CalculateRelease(1);
        releaseStepper_.SetValue((Q88)currentLevel_);
        
        state_ = State::RELEASE;
    }
    
    // return a scaling factor
    Q08 GetNextEnvelope()
    {
        uint8_t retVal = 0;
        
        if (state_ == State::ATTACK)
        {
            // Convert to decimal value
            retVal = (uint8_t)attackStepper_;
            
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
            retVal = (uint8_t)decayStepper_;
            
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
            // caller invokes StartRelease()
            retVal = sustainLevel_;
        }
        else if (state_ == State::RELEASE)
        {
            // Convert to decimal value
            retVal = (uint8_t)releaseStepper_;
            
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
        
        // Cache for times when Decay begins before Sustain was reached
        currentLevel_ = retVal;
        
        return (Q08)retVal;
    }
    
    void Reset()
    {
        state_ = State::DONE;
        
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
        uint16_t durationMsUse = durationMs ? durationMs : 1;
        
        double stepSize =
            (double)(deltaY) / 
            (((double)sampleRate_ / 1000.0) * (double)durationMsUse);
            
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
    
    void CalculateRelease(uint8_t useCurrentLevel = 0)
    {
        uint8_t level = useCurrentLevel ? currentLevel_ : sustainLevel_;
        
        // Just go linear down
        double stepSize = CalculateLinearStepSize(level - BOTTOM_VALUE,
                                                  releaseDurationMs_);
        
        // Store calculated value
        releaseStepper_.SetStepSize(stepSize);
    }
    
    void SetConstantValues()
    {
        attackStepper_.SetLimitUpper((Q88)TOP_VALUE);
        releaseStepper_.SetLimitLower((Q88)BOTTOM_VALUE);
    }
    
    void SetStepwiseInitialValues()
    {
        attackStepper_.SetValue(attackStepperInitialValue_);
        
        decayStepper_.SetValue(decayStepperInitialValue_);
        decayStepper_.SetLimitLower((Q88)sustainLevel_);
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
    State   state_;
    uint8_t currentLevel_ = 0;
    
    // Cached raw values
    uint16_t sampleRate_ = DEFAULT_SAMPLE_RATE;
    
    // Attack values
    uint16_t                attackDurationMs_          = DEFAULT_ATTACK_DURATION_MS;
    FixedPointStepper<Q88>  attackStepper_;
    Q88                     attackStepperInitialValue_ = BOTTOM_VALUE;

    // Decay values
    uint16_t                decayDurationMs_          = DEFAULT_DECAY_DURATION_MS;
    FixedPointStepper<Q88>  decayStepper_;
    Q88                     decayStepperInitialValue_ = TOP_VALUE;

    // Sustain values
    uint8_t sustainLevelPct_ = DEFAULT_SUSTAIN_LEVEL_PCT;
    uint8_t sustainLevel_    = ((double)DEFAULT_SUSTAIN_LEVEL_PCT / 100.0) *
                               TOP_VALUE;
    
    // Release values
    uint16_t                releaseDurationMs_ = DEFAULT_RELEASE_DURATION_MS;
    FixedPointStepper<Q88>  releaseStepper_;
};


#endif  // __SIGNAL_ENVELOPE_ADSR__






