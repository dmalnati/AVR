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
    static const uint8_t STEP_COUNT = 21;

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



/*


Holds many callback objects

Does not check its own time, relies on external prompting

Synchronous in the sense that it's driven by a signal
- each Tick() increments step

Has no idea about time at all

*/

enum class SignalMode : uint8_t {
    MODE_UP_THEN_DOWN,
    MODE_FOREVER
};

template <uint8_t COUNT_SIGNAL_EVENT_HANDLER>
class SynchronousSignalEventDistributor
{
public:
    static const SignalMode MODE_DEFAULT = SignalMode::MODE_UP_THEN_DOWN;

    SynchronousSignalEventDistributor(
        SignalSource *ss,
        SignalMode    mode = MODE_DEFAULT)
    : signalStepCount_(ss->GetStepCount())
    , mode_(mode)
    , ssr_(ss)
    {
        Reset();
    }

    void AddSignalEventHandler(SignalEventHandler *seh)
    {
        signalEventHandlerList_.Push(seh);
    }
    
    void Reset()
    {
        stepCountCurrent_ = 0;
        stepDirection_    = 1;
        done_             = 0;
        
        DistributeLogicLevelToAllHandlers(0);
    }
    
    void SetMode(SignalMode mode)
    {
        mode_ = mode;
    }
    
    void Tick()
    {
        if (!done_)
        {
            uint8_t logicLevel = ssr_.GetNextLogicLevel();
            
            DistributeLogicLevelToAllHandlers(logicLevel);
        }
    }

    void ChangeStep()
    {
        stepCountCurrent_ += stepDirection_;
        
        if (stepCountCurrent_ >= signalStepCount_)
        {
            if (stepDirection_ == 1)
            {
                // Hit the upper threshold, turn around.
                // Do the last step again.
                stepCountCurrent_ = signalStepCount_ - 1;
                
                stepDirection_ = -1;
            }
            else // (stepDirection_ == -1)
            {
                if (mode_ == SignalMode::MODE_UP_THEN_DOWN)
                {
                    done_ = 1;
                }
                else // (mode == MODE_FOREVER)
                {
                    // Hit the lower threshold, turn around.
                    // Do the last step again.
                    stepCountCurrent_ = 0;
                    
                    stepDirection_ = 1;
                }
            }
        }
        
        ssr_.SetStep(stepCountCurrent_);
    }
    
    uint8_t IsDone()
    {
        return done_;
    }
    
    
private:
    void DistributeLogicLevelToAllHandlers(uint8_t logicLevel)
    {
        for (uint8_t i = 0; i < signalEventHandlerList_.Size(); ++i)
        {
            signalEventHandlerList_[i]->OnSignalEvent(logicLevel);
        }
    }
    
    
    uint8_t     signalStepCount_;
    uint8_t     stepCountCurrent_;
    int8_t      stepDirection_;
    SignalMode  mode_;
    uint8_t     done_;
    
    SignalSourceReader          ssr_;
    Queue<SignalEventHandler*, COUNT_SIGNAL_EVENT_HANDLER>
                                signalEventHandlerList_;
};








//////////////////////////////////////////////////////////////////////
//
// Knows when it's time to move to next signal pattern by being a
// hi-res clock
//
//////////////////////////////////////////////////////////////////////

template <uint8_t COUNT_LED, uint8_t COUNT_PHASE_OFFSET>
class PhaseOffsetSignalDistributor
: private IdleTimeEventHandler
{
public:
    static const uint8_t DEFAULT_PHASE_OFFSET = 0;

    PhaseOffsetSignalDistributor()
    : step_(0)
    , stepMaxReached_(0)
    {
        // Nothing to do
    }
    
    void AddSignalEventHandler(
        SignalEventHandler *seh,
        uint16_t            phaseOffset = DEFAULT_PHASE_OFFSET)
    {
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
            (uint8_t)((double)phaseOffset / (double)360 *
                      (double)signalSource_.GetStepCount() * 2.0);
        
        // Check if there is already a group set up for this phase
        PhaseData *pd = FindPhaseDataByStepOffset(stepOffset);
        
        if (!pd)
        {
            // Not found -- need to create the group
            // Create the group
            pd = phaseDataList_.PushNew(&signalSource_, stepOffset);
            
            // Add the phase group to the pending list, which
            // will be detected later
            
            // Make sure allocation actually worked
            if (pd)
            {
                phaseDataPendingList_.Push(pd);
            }
        }
        
        // Add handler
        // Make sure allocation actually worked
        if (pd)
        {
            pd->ssed.AddSignalEventHandler(seh);
        }
    }
    
    
    void PulseWithMode(uint32_t pulseDurationMs, SignalMode mode)
    {
        // Stop whatever was going on before and get back to initial state
        Stop();
        
        // Set the distributors to operate in a given mode
        SetDistributorMode(mode);
        
        // Calculate total microseconds this pulse will be active
        uint32_t pulseDurationUs = pulseDurationMs * 1000;
        
        // Assuming up-then-down, calculate duration of each step
        pulseStepDurationUs_ =
            pulseDurationUs / (signalSource_.GetStepCount() * 2);
        
        // Note the current time
        timeLastUs_ = PAL.Micros();
        
        // Begin handling idle callbacks to drive signal distribution
        RegisterForIdleTimeEvent();
    }
    
    void SetDistributorMode(SignalMode mode)
    {
        // Set active list
        for (uint8_t i = 0; i < phaseDataActiveList_.Size(); ++i)
        {
            phaseDataActiveList_[i]->ssed.SetMode(mode);
        }
        
        // Set pending list
        for (uint8_t i = 0; i < phaseDataPendingList_.Size(); ++i)
        {
            phaseDataPendingList_[i]->ssed.SetMode(mode);
        }
    }

    
    void PulseOnce(uint32_t pulseDurationMs)
    {
        PulseWithMode(pulseDurationMs, SignalMode::MODE_UP_THEN_DOWN);
    }
    
    void PulseForever(uint32_t pulseDurationMs)
    {
        PulseWithMode(pulseDurationMs, SignalMode::MODE_FOREVER);
    }
    
    void Stop()
    {
        // Stop getting idle callbacks
        DeRegisterForIdleTimeEvent();
        
        ResetDistributors();
        
        // Reset state keeping for when we start again
        step_           = 0;
        stepMaxReached_ = 0;
    }
    
    void OnPulseComplete()
    {
        Stop();
    }
    
    
private:
    class PhaseData
    {
    public:
        PhaseData(SignalSource *ss, uint8_t stepOffset)
        : stepOffset(stepOffset)
        , ssed(ss)
        {
            // Nothing to do
        }
        
        uint8_t  stepOffset;
        
        // This leads to probably more memory than required.
        // Effectively the amount of storage SSED (total across all PhaseDatas)
        // is COUNT_LED * COUNT_PHASE_OFFSET
        SynchronousSignalEventDistributor<COUNT_LED>  ssed;
    };
    
    
    
    void OnIdleTimeEvent()
    {
        uint32_t timeNowUs = PAL.Micros();
        
        // Check if the last step duration has expired
        if ((timeNowUs - timeLastUs_) >= pulseStepDurationUs_)
        {
            // Record current time for next evaluation.
            timeLastUs_ = timeNowUs;
            
            // Cause all Phases to take a step to the new signal data
            for (uint8_t i = 0; i < phaseDataActiveList_.Size(); ++i)
            {
                phaseDataActiveList_[i]->ssed.ChangeStep();
            }
            
            // Keep track of how many steps you are in, up to
            // a maximum
            if (!stepMaxReached_)
            {
                ++step_;
                
                if (step_ >= (signalSource_.GetStepCount() * 2))
                {
                    stepMaxReached_ = 1;
                }
            }
            
            // Check for any pending phases which need to get moved
            // to be active.
            for (uint8_t i = 0; i < phaseDataPendingList_.Size(); /* nothing */)
            {
                if (stepMaxReached_ ||
                    phaseDataPendingList_[i]->stepOffset <= step_)
                {
                    PhaseData *pd = phaseDataPendingList_[i];
                    
                    phaseDataPendingList_.Remove(pd);
                    phaseDataActiveList_.Push(pd);
                }
                else
                {
                    ++i;
                }
            }
        }

        // Iterate over Phases to tick them
        uint8_t doneCount = 0;
        for (uint8_t i = 0; i < phaseDataActiveList_.Size(); ++i)
        {
            if (phaseDataActiveList_[i]->ssed.IsDone())
            {
                ++doneCount;
            }
            else
            {
                phaseDataActiveList_[i]->ssed.Tick();
            }
        }
        
        // Check if every phased element has completed
        // and that there are no pending elements
        if (doneCount == phaseDataActiveList_.Size() &&
            !phaseDataPendingList_.Size())
        {
            OnPulseComplete();
        }
    }
    
    void ResetDistributors()
    {
        // Move the contents of the active container into
        // the pending list
        while (phaseDataActiveList_.Size())
        {
            PhaseData *pd = NULL;
            phaseDataActiveList_.Pop(pd);
            
            phaseDataPendingList_.Push(pd);
        }
        
        // Reset everything, which is now in the pending list
        for (uint8_t i = 0; i < phaseDataPendingList_.Size(); ++i)
        {
            phaseDataPendingList_[i]->ssed.Reset();
        }
    }
    
    
    PhaseData *FindPhaseDataByStepOffset(uint8_t stepOffset)
    {
        PhaseData *retVal = NULL;
        
        // Search active list
        for (uint8_t i = 0; i < phaseDataActiveList_.Size(); ++i)
        {
            if (phaseDataActiveList_[i]->stepOffset == stepOffset)
            {
                retVal = phaseDataActiveList_[i];
                
                break;
            }
        }
        
        // Search pending list
        for (uint8_t i = 0; i < phaseDataPendingList_.Size(); ++i)
        {
            if (phaseDataPendingList_[i]->stepOffset == stepOffset)
            {
                retVal = phaseDataPendingList_[i];
                
                break;
            }
        }
        
        return retVal;
    }
    
    
    // State keeping for pulsing
    uint32_t pulseStepDurationUs_;
    uint32_t timeLastUs_;
    
    uint8_t  step_;
    uint8_t  stepMaxReached_;
    
    // Signal synchronizing structures
    SignalSource                               signalSource_;
    ListInPlace<PhaseData, COUNT_PHASE_OFFSET> phaseDataList_;
    Queue<PhaseData *, COUNT_PHASE_OFFSET>     phaseDataActiveList_;
    Queue<PhaseData *, COUNT_PHASE_OFFSET>     phaseDataPendingList_;
};
 
 
 
 
//////////////////////////////////////////////////////////////////////
//
// LED Toggler
//
//////////////////////////////////////////////////////////////////////


class LEDToggler
: public SignalEventHandler
{
public:
    LEDToggler(uint8_t pin)
    : pin_(pin)
    {
        SetPin(pin_);
    }
    
    void SetPin(uint8_t pin)
    {
        pin_ = pin;
        
        PAL.PinMode(pin_, OUTPUT);
    }

    virtual void OnSignalEvent(uint8_t logicLevel)
    {
        PAL.DigitalWrite(pin_, logicLevel);
    }
    
private:
    uint8_t pin_;
};


//////////////////////////////////////////////////////////////////////
//
// LED Fader interface and instances
//
//////////////////////////////////////////////////////////////////////



class LEDFaderControl
{
public:
    virtual void FadeOnce(uint32_t    fadeDurationMs = 1000) = 0;
    virtual void FadeForever(uint32_t fadeDurationMs = 1000) = 0;
    virtual void Stop()                                      = 0;
};



template <uint8_t COUNT_LED, uint8_t COUNT_PHASE_OFFSET>
class LEDFader
: public LEDFaderControl
{
public:
    static const uint32_t DEFAULT_FADE_DURATION_MS = 1000;
    
    ~LEDFader()
    {
        Stop();
    }

    void AddLED(uint8_t pin, uint16_t phaseOffset = 0)
    {
        // Push in place
        LEDToggler *lt = ledTogglerList_.PushNew(pin);
        
        if (lt)
        {
            posd_.AddSignalEventHandler(lt, phaseOffset);
        }
    }
    
    virtual
    void FadeOnce(uint32_t fadeDurationMs = DEFAULT_FADE_DURATION_MS)
    {
        posd_.PulseOnce(fadeDurationMs);
    }
    
    virtual
    void FadeForever(uint32_t fadeDurationMs = DEFAULT_FADE_DURATION_MS)
    {
        posd_.PulseForever(fadeDurationMs);
    }
    
    virtual
    void Stop()
    {
        posd_.Stop();
    }
    
private:
    
    PhaseOffsetSignalDistributor<COUNT_LED, COUNT_PHASE_OFFSET>  posd_;
    ListInPlace<LEDToggler, COUNT_LED>                           ledTogglerList_;
};
 
 
 
 
template <uint8_t COUNT_LED_TRIPLET>
class LEDFaderRGB
: public LEDFaderControl
{
public:
    void AddLED(uint8_t pinRed,
                uint8_t pinGreen,
                uint8_t pinBlue,
                uint16_t phaseOffset = 0)
    {
        ledFader_.AddLED(pinRed,   (phaseOffset +  45) % 360);
        ledFader_.AddLED(pinGreen, (phaseOffset + 225) % 360);
        ledFader_.AddLED(pinBlue,  (phaseOffset +   0) % 360);
    }
    
    virtual
    void FadeOnce(uint32_t fadeDurationMs = 1000)
    {
        ledFader_.FadeOnce(fadeDurationMs);
    }
    
    virtual
    void FadeForever(uint32_t fadeDurationMs = 1000)
    {
        ledFader_.FadeForever(fadeDurationMs);
    }
    
    virtual
    void Stop()
    {
        ledFader_.Stop();
    }

private:

    LEDFader<COUNT_LED_TRIPLET * 3,
             COUNT_LED_TRIPLET * 3 * 3> ledFader_;
};

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 







#endif  // __LED_FADER_H__








