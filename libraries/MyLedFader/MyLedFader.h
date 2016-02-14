#ifndef __MY_LED_FADER_H__
#define __MY_LED_FADER_H__


#include "PAL.h"
#include "EvmEventHandler.h"


class MyLedFader : private IdleTimeEventHandler
{
public:
    MyLedFader(uint8_t pin)
    : pin_(pin)
    , step_(0)
    , stepDir_(1)
    , onOffState_(0)
    {
        PAL.PinMode(pin_, OUTPUT);
    }

    const uint16_t STEP_DURATION = 50000;
    const uint8_t  STEP_MAX      = 11;

    void Fade()
    {
        // Register as idle callback
        RegisterForIdleTimeEvent();

        // Set up for next (first) step
        bool reset = true;
        SetStateForNextStep(reset);
    }

private:
    void SetStateForNextStep(bool reset = false)
    {
        // First part of phase is counting up.
        // The second is counting down.
        // When it hits zero on count down, the whole cycle is over.
        
        if (reset)
        {
            step_    = 0;
            stepDir_ = 1;
        }

        if (stepDir_ == 1)
        {
            // In increasing mode
            ++step_;

            // Check if you have now moved beyond the final step upward
            if (step_ >= STEP_MAX)
            {
                // Yes, turn around.
                step_ = STEP_MAX - 2;
                stepDir_ = -1;
            }
        }
        else if (stepDir_ == -1)
        {
            // In decreasing mode
            --step_;

            // Check if you have now moved beyond the final step downward.
            // Since 0 is the lowest, wraparound will occur and you detect that
            if (step_ >= STEP_MAX)
            {
                step_ = 1;
                stepDir_ = 1;
            }
        }

        // Set the on/off frequency counters
        SetStepQuota();
        onOffState_ = 0;

        // Record first step time start
        timeStepStart_    = PAL.Micros();
        timeStepDuration_ = STEP_DURATION;
    }

    void OnIdleTimeEvent()
    {
        // Has current step expired?
        uint32_t timeNow = PAL.Micros();

        if ((timeNow - timeStepStart_) >= timeStepDuration_)
        {
            // yes, it expired.  Go to next step.
            SetStateForNextStep();
        }

        ApplyIntensity();
    }

    void ApplyIntensity()
    {
        // check if quota for this on/off state has run out
        if (!onOffState__quota_[onOffState_])
        {
            // move to next state
            onOffState_ ^= 0x01;

            // if just moved back to 0th index, new quota required
            if (onOffState_ == 0)
            {
                SetStepQuota();
            }
        }

        // only apply if there is some quota for this state
        if (onOffState__quota_[onOffState_])
        {
            // The value of the pin should be the opposite of the index
            // 0th index represents on
            // 1st index represents off
            PAL.DigitalWrite(pin_, !onOffState_);
            --onOffState__quota_[onOffState_];
        }
    }

    // Only supported within pre-defined indexes
    void SetStepQuota()
    {
        static uint8_t settings[][2] = {
            { 0, 1 },   // 0   %
            { 1, 9 },   // 10  %
            { 1, 4 },   // 20  %
            { 3, 7 },   // 30  %
            { 2, 3 },   // 40  %
            { 1, 1 },   // 50  %
            { 3, 2 },   // 60  %
            { 7, 3 },   // 70  %
            { 4, 1 },   // 80  %
            { 9, 1 },   // 90  %
            { 1, 0 }    // 100 %
        };

        onOffState__quota_[0] = settings[step_][0];
        onOffState__quota_[1] = settings[step_][1];
    }

    
    uint8_t pin_;
    uint8_t onOffState_;
    uint8_t duration_;
    uint8_t step_;
    int8_t  stepDir_;
    uint32_t timeStepStart_;
    uint32_t timeStepExpire_;
    uint32_t timeStepDuration_;

    uint8_t onOffState__quota_[2];
};


#endif  // __MY_LED_FADER_H__


















