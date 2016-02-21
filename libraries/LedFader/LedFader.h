#ifndef __LED_FADER_H__
#define __LED_FADER_H__


#include <util/atomic.h>

#include <stdint.h>

#include <PAL.H>
#include <Container.h>
#include <IdleTimeEventHandler.h>









//////////////////////////////////////////////////////////////////////
//
// SignalSource and Signal Reader
//
// SignalSource:
//
//
// SignalReader:
//
//
//
//////////////////////////////////////////////////////////////////////

 
//
// Configure it for period, steps.  Not repeating.
// That's someone else's job to implement.
//
// This class doesn't do very much.
// Someone configures it.
// Other objects are given a reference and read from it without knowing much.
//
class SignalSource
{
public:
    uint8_t GetStepCount() const { return STEP_COUNT; }
    
    void GetSignalPatternAtStep(uint8_t step, uint8_t onOffCount[2])
    {
        // 0-indexed
        onOffCount[0] = step__onOffCount_[step][0];
        onOffCount[1] = step__onOffCount_[step][1];
    }
    
private:
    static const uint8_t STEP_COUNT = 20;

    static uint8_t step__onOffCount_[][2];
};
 


 
//
// Utility class
//
// Basically keeps state and implements the tricky parts of making use of the
// data that the SignalGenerator creates.
//
// This and the SignalGenerator are friends and must be implemented together.
//
class SignalSourceReader
{
public:
    SignalSourceReader(SignalSource *ss, uint8_t step = DEFAULT_STEP)
    : ss_(ss)
    , step_(step)
    , onOffState_(0)
    {
        SetStep(step_);
    }
    
    void SetStep(uint8_t step)
    {
        step_ = step;
        
        ss_->GetSignalPatternAtStep(step_, signalPatternToRefreshFrom_);
        
        RefreshOnOffStateQuota();
        
        onOffState_ = 0;
    }
    
    void MoveToNextStep()
    {
        ++step_;
        
        if (step_ >= ss_->GetStepCount() || step_ == 0)
        {
            step_ = 0;
        }
        
        SetStep(step_);
    }
    
    void MoveToPrevStep()
    {
        --step_;
        
        if (step_ >= ss_->GetStepCount())
        {
            step_ = ss_->GetStepCount() - 1;
        }
        
        SetStep(step_);
    }
    
    //
    // onOffState_ is both:
    // - an index into the quota system
    // - the opposite of the logic level which applies at that index
    //
    // That is, idx=0 refers to the logic level 1
    //
    // Relies on that when quotas are refreshed, at least one of the on/off
    // states has a positive (non-zero) quota.
    //
    uint8_t GetNextLogicLevel()
    {
        uint8_t retVal; // init not necessary, retVal set in every branch
        
        if (onOffState_ == 0)
        {
            // Check if any remaining quota for on state
            if (onOffState__quota_[onOffState_])
            {
                // Note logic level
                retVal = 1;
                
                // Decrement quota
                --onOffState__quota_[onOffState_];
            }
            else
            {
                onOffState_ = 1;
             
                // Check if any quota available for off state
                if (onOffState__quota_[onOffState_])
                {
                    // Note logic level
                    retVal = 0;
                    
                    // Decrement quota
                    --onOffState__quota_[onOffState_];
                }
                else
                {
                    // Go back to on state and refresh quota
                    onOffState_ = 0;
                    
                    RefreshOnOffStateQuota();

                    // Note logic level
                    retVal = 1;
                    
                    // Decrement quota
                    --onOffState__quota_[onOffState_];
                }
            }
        }
        else // (onOffState_ == 1)
        {
            // Check if any remaining quota for on state
            if (onOffState__quota_[onOffState_])
            {
                // Note logic level
                retVal = 0;
                
                // Decrement quota
                --onOffState__quota_[onOffState_];
            }
            else
            {
                // Go back to beginning
                onOffState_ = 0;
                
                // Refresh quota since we're going back to beginning
                RefreshOnOffStateQuota();
             
                // Check if any quota available for on state
                if (onOffState__quota_[onOffState_])
                {
                    // Note logic level
                    retVal = 1;
                    
                    // Decrement quota
                    --onOffState__quota_[onOffState_];
                }
                else
                {
                    // Go back to off state
                    onOffState_ = 1;
                    
                    // Note logic level
                    retVal = 0;
                    
                    // Decrement quota
                    --onOffState__quota_[onOffState_];
                }
            }
        }
        
        return retVal;
    }
    
private:
    static const uint8_t DEFAULT_STEP = 0;

    void RefreshOnOffStateQuota()
    {
        onOffState__quota_[0] = signalPatternToRefreshFrom_[0];
        onOffState__quota_[1] = signalPatternToRefreshFrom_[1];
    }
    
 
    SignalSource *ss_;
    uint8_t       step_;
    uint8_t       onOffState_;
    uint8_t       signalPatternToRefreshFrom_[2];
    uint8_t       onOffState__quota_[2];
};




















//////////////////////////////////////////////////////////////////////
//
// Signal Event Handler Interface
//
//////////////////////////////////////////////////////////////////////

class SignalEventHandler
{
public:
    virtual void OnSignalEvent(uint8_t logicLevel) = 0;
};




//////////////////////////////////////////////////////////////////////
//
// Knows when it's time to move to next signal pattern by being a
// hi-res clock
//
//////////////////////////////////////////////////////////////////////

class PhaseOffsetSignalDistributor
: private IdleTimeEventHandler
{
public:
    static const uint8_t DEFAULT_PHASE_OFFSET = 0;

    PhaseOffsetSignalDistributor()
    : stepMaster_(0)
    {
        // Nothing to do
    }
    
    virtual ~PhaseOffsetSignalDistributor()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            for (uint8_t i = 0; i < phaseGroupList_.Size(); ++i)
            {
                delete phaseGroupList_[i];
            }
        }
    }
    
    void AddSignalEventHandler(
        SignalEventHandler *seh,
        uint16_t            phaseOffset = DEFAULT_PHASE_OFFSET)
    {
        // Check if there is already a group set up for this phase
        PhaseGroup *pg = FindPhaseGroupByPhaseOffset(phaseOffset);
        
        if (!pg)
        {
            // Not found -- need to create new group
            
            // We know the number of steps a signal source makes available.
            // From that, we convert the phaseOffset into a step in the
            // signal sequence.
        
            // Constrain input to acceptable range
            if (phaseOffset >= 360)
            {
                phaseOffset = 0;
            }

            // Calculate how many steps this phaseOffset translates into.
            //
            // Casting result to int truncates any floating remainder giving a
            // zero-indexed number.
            uint8_t stepOffset =
                (uint8_t)((float)phaseOffset / (float)360 *
                          (float)signalSource_.GetStepCount());
            
            // Calculate what step this handler should be on in terms of the
            // master.
            uint8_t step = stepMaster_ + stepOffset;
            
            if (step >= signalSource_.GetStepCount())
            {
                step = step - signalSource_.GetStepCount();
            }
            
            // Create the group
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                pg = new PhaseGroup(&signalSource_, step, phaseOffset);
            }
            
            // Add the phase group to the list
            phaseGroupList_.Push(pg);
        }
        
        // Add handler
        pg->signalEventHandlerList.Push(seh);
    }

    
    
    
    
    void Stop()
    {
        // Stop getting idle callbacks
        DeRegisterForIdleTimeEvent();
        
        // Distribute logicLevel 0 to all pins
        DistributeLogicLevelToAllHandlers(0);
        
        // Reset back to initial state
        ResetState();
    }
    
    void PulseOnce(uint32_t pulseDurationMs)
    {
        PAL.PinMode(11, OUTPUT);
        PAL.DigitalWrite(11, HIGH);
        
        // Stop whatever was going on before and get back to initial state
        Stop();
        
        // Calculate total microseconds this pulse will be active
        uint32_t pulseDurationUs = pulseDurationMs * 1000;
        
        // Assuming up-then-down, calculate duration of each step
        pulseStepDurationUs_ =
            pulseDurationUs / (signalSource_.GetStepCount() * 2);
        
        // Calculate step count to the halfway point
        pulseStepCountHalfway_ = (pulseDurationUs / pulseStepDurationUs_) / 2;
        
        // Keep a modifiable counter for steps remaining before half complete
        pulseStepCountHalfwayRemaining_ = pulseStepCountHalfway_;
        
        // Note the current time
        timeLastUs_ = PAL.Micros();
        
        // Note upward step direction
        upDownDirection_ = 1;
        
        // Begin handling idle callbacks to drive signal distribution
        RegisterForIdleTimeEvent();
    }
    
    
    
    void PulseForever() { }
    
    
    
private:
    
    
    struct PhaseGroup
    {
        PhaseGroup(SignalSource *signalSource,
                   uint8_t       step,
                   uint16_t      phaseOffset)
        : ssr(signalSource, step)
        , phaseOffset(phaseOffset)
        , stepToResetTo(step)
        {
            // Nothing to do
        }
        
        SignalSourceReader           ssr;
        uint16_t                     phaseOffset;
        uint8_t                      stepToResetTo;
        Queue<SignalEventHandler *>  signalEventHandlerList;
    };
    
    
    void OnPulseComplete()
    {
        PAL.DigitalWrite(11, LOW);
        Stop();
    }
    
    void ResetState()
    {
        stepMaster_ = 0;
        
        for (uint8_t i = 0; i < phaseGroupList_.Size(); ++i)
        {
            PhaseGroup *pg = phaseGroupList_[i];
            
            pg->ssr.SetStep(pg->stepToResetTo);
        }
    }
    
    void DistributeLogicLevelToAllHandlers(uint8_t logicLevel)
    {
        for (uint8_t i = 0; i < phaseGroupList_.Size(); ++i)
        {
            PhaseGroup *pg = phaseGroupList_[i];

            // Distribute the logic level to the whole group
            for (uint8_t j = 0; j < pg->signalEventHandlerList.Size(); ++j)
            {
                pg->signalEventHandlerList[j]->OnSignalEvent(logicLevel);
            }
        }
    }
    
    
    
    
    void OnIdleTimeEvent()
    {
        uint8_t timeToMoveToNextStep = 0;
        uint8_t pulseComplete        = 0;
        
        uint32_t timeNowUs = PAL.Micros();
        
        // Check if the last step duration has expired
        if ((timeNowUs - timeLastUs_) >= pulseStepDurationUs_)
        {
            // Time to move to the next step
            timeToMoveToNextStep = 1;
            
            // First, check if we need to change directions or even if the
            // pulse is complete
            --pulseStepCountHalfwayRemaining_;
            
            if (!pulseStepCountHalfwayRemaining_)
            {
                // Half of the steps have been used.  Was it the first half
                // or the second half?
                if (upDownDirection_ == 1)
                {
                    // First half expired.  Time to count downward.
                    
                    upDownDirection_                = -1;
                    pulseStepCountHalfwayRemaining_ = pulseStepCountHalfway_;
                }
                else // (upDownDirection_ == -1)
                {
                    // The second half expired.  This is the end.

                    pulseComplete = 1;
                }
            }
            
            // Finished with calculation, record current time for next evaluation.
            timeLastUs_ = timeNowUs;
        }
        
        
        // If the pulse isn't complete, then time to get and distribute the
        // next logic level.
        if (!pulseComplete)
        {
            // Maintain the master step
            if (timeToMoveToNextStep)
            {
                stepMaster_ += upDownDirection_;
                
                // You wind up above the max whether counting up, or
                // counting down and wrapping around zero.
                if (stepMaster_ >= signalSource_.GetStepCount())
                {
                    stepMaster_ = 0;
                }
            }
        
            DistributeLogicLevel(timeToMoveToNextStep, upDownDirection_);
        }
        else
        {
            OnPulseComplete();
        }
    }
    
    void DistributeLogicLevel(uint8_t timeToMoveToNextStep,
                              uint8_t stepDirection)
    {
        // Determine and distribute a logic level to each group
        for (uint8_t i = 0; i < phaseGroupList_.Size(); ++i)
        {
            PhaseGroup *pg = phaseGroupList_[i];

            // If time to move to next step, do so
            if (timeToMoveToNextStep)
            {
                // Tell the signal reader to advance to the next signal point
                if (stepDirection == 1)
                {
                    pg->ssr.MoveToNextStep();
                }
                else // (stepDirection == -1)
                {
                    pg->ssr.MoveToPrevStep();
                }
            }
            
            // Determine the logic level which applies to the whole group
            uint8_t logicLevel = pg->ssr.GetNextLogicLevel();
            
            // Distribute the logic level to the whole group
            for (uint8_t j = 0; j < pg->signalEventHandlerList.Size(); ++j)
            {
                pg->signalEventHandlerList[j]->OnSignalEvent(logicLevel);
            }
        }
    }
    
    PhaseGroup *FindPhaseGroupByPhaseOffset(uint16_t phaseOffset)
    {
        PhaseGroup *retVal = NULL;
        
        for (uint8_t i = 0; i < phaseGroupList_.Size(); ++i)
        {
            if (phaseGroupList_[i]->phaseOffset == phaseOffset)
            {
                retVal = phaseGroupList_[i];
                
                break;
            }
        }
        
        return retVal;
    }

    
    // State keeping for pulsing
    uint8_t  stepMaster_;
    uint16_t pulseStepCountHalfway_;
    uint16_t pulseStepCountHalfwayRemaining_;
    uint32_t pulseStepDurationUs_;
    uint32_t timeLastUs_;
    uint8_t  upDownDirection_;
    
    // Signal synchronizing structures
    SignalSource        signalSource_;
    Queue<PhaseGroup *> phaseGroupList_;
};



 


 
 
 
 
 
 
 
 
 
 
 
 
//////////////////////////////////////////////////////////////////////
//
// LED Fader
//
//////////////////////////////////////////////////////////////////////

 
 
class LEDFader
{
public:
    static const uint32_t DEFAULT_FADE_DURATION_MS = 1000;

    LEDFader() { }
    ~LEDFader()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            for (uint8_t i = 0; i < ledTogglerList_.Size(); ++i)
            {
                delete ledTogglerList_[i];
            }
        }
    }

    void AddLED(
        uint8_t pin,
        uint16_t phaseOffset = PhaseOffsetSignalDistributor::DEFAULT_PHASE_OFFSET)
    {
        LEDToggler *lt = NULL;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            lt = new LEDToggler(pin);
        }

        ledTogglerList_.Push(lt);
        
        posd_.AddSignalEventHandler(lt, phaseOffset);
    }
    
    void FadeOnce(uint32_t pulseDurationMs = DEFAULT_FADE_DURATION_MS)
    {
        posd_.PulseOnce(pulseDurationMs);
    }
    
    void Stop()
    {
        posd_.Stop();
    }
    
private:

    class LEDToggler
    : public SignalEventHandler
    {
    public:
        LEDToggler(uint8_t pin)
        : pin_(pin)
        {
            PAL.PinMode(pin_, OUTPUT);
        }
    
        virtual void OnSignalEvent(uint8_t logicLevel)
        {
            PAL.DigitalWrite(pin_, logicLevel);
        }
        
    private:
        uint8_t pin_;
    };


    
    PhaseOffsetSignalDistributor  posd_;
    Queue<LEDToggler *>           ledTogglerList_;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 







#endif  // __LED_FADER_H__








