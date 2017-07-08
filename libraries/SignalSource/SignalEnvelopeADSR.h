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
            retVal = (uint8_t)attackVal_;
            
            // Move to next value
            attackVal_ += attackStepSize_;
            
            // Check if time to move to decay for next sample
            if (retVal >= TOP_VALUE)
            {
                retVal = TOP_VALUE;
                
                state_ = State::DECAY;
            }
        }
        else if (state_ == State::DECAY)
        {
            // Convert to decimal value
            retVal = (uint8_t)decayVal_;
            
            // Move to next value
            decayVal_ -= decayStepSize_;
            
            // Check if time to move to sustain for next sample
            if (retVal <= sustainLevel_)
            {
                retVal = sustainLevel_;
                
                state_ = State::SUSTAIN;
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
            retVal = (uint8_t)releaseVal_;
            
            // Move to next value
            releaseVal_ -= releaseStepSize_;
            
            // Check if time to move to final state
            // this would be in the form of a wrap around
            if (retVal >= TOP_VALUE)
            {
                retVal = BOTTOM_VALUE;
                
                state_ = State::DONE;
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
        attackStepSize_ = stepSize;
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
        decayStepSize_ = stepSize;
    }
    
    void CalculateRelease()
    {
        // Just go linear down
        double stepSize = CalculateLinearStepSize(sustainLevel_ - BOTTOM_VALUE,
                                                  releaseDurationMs_);
        
        // Store calculated value
        releaseStepSize_ = stepSize;
    }
    
    void SetStepwiseInitialValues()
    {
        attackVal_  = (double)BOTTOM_VALUE;
        decayVal_   = (double)TOP_VALUE;
        releaseVal_ = (double)sustainLevel_;
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
    Q1616    attackVal_          {(double)BOTTOM_VALUE};
    Q1616    attackStepSize_     {(double)DEFAULT_STEP_SIZE};
    
    // Decay values
    uint16_t decayDurationMs_ = DEFAULT_DECAY_DURATION_MS;
    Q1616    decayVal_          {(double)TOP_VALUE};
    Q1616    decayStepSize_     {(double)DEFAULT_STEP_SIZE};

    // Sustain values
    uint8_t sustainLevelPct_ = DEFAULT_SUSTAIN_LEVEL_PCT;
    uint8_t sustainLevel_    = ((double)DEFAULT_SUSTAIN_LEVEL_PCT / 100.0) *
                               TOP_VALUE;
    
    // Release values
    uint16_t releaseDurationMs_ = DEFAULT_RELEASE_DURATION_MS;
    Q1616    releaseVal_        {(double)sustainLevel_};
    Q1616    releaseStepSize_   {(double)DEFAULT_STEP_SIZE};
};


#endif  // __SIGNAL_ENVELOPE_ADSR__






