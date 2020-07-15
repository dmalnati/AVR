#ifndef __RGB_LED_PWM_CONTROLLER_H__
#define __RGB_LED_PWM_CONTROLLER_H__


#include "Timer1.h"
#include "Timer2.h"
#include "Log.h"


// Timer1:A = Red      = 15
// Timer1:B = Green    = 16
// Timer2:A = (Unused) = 17
// Timer2:B = Blue     =  5


// Designed to simplify accessing PWM for RGB LEDs.
// Timer details hidden.
//
// Supports all the way on, all the way off, and full PWM range between.
//
// Init() // just once
// Start()
//   Set Red/Green/Blue for instant effect
// Stop()
//   All LEDs turn off
//
// Setting/Getting RGB values while stopped caches values.
// Starting restores cached values.
//

class RgbLedPwmController
{
public:

    RgbLedPwmController()
    : tcLedRed_(Timer1::GetTimerChannelA())
    , tcLedGreen_(Timer1::GetTimerChannelB())
    , tcLedBlue_(Timer2::GetTimerChannelB())
    , redValCached_(0)
    , greenValCached_(0)
    , blueValCached_(0)
    , running_(0)
    {
        // Nothing to do
    }

    ~RgbLedPwmController()
    {
        Stop();
    }


    // Do this to put timers in known state after all startup routines
    // have completed, as the Arduino system mucks around too much.
    void Init()
    {
        Stop();

        Timer1::SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
        Timer1::SetTimerMode(Timer1::TimerMode::FAST_PWM_8_BIT);
        
        Timer2::SetTimerPrescaler(Timer2::TimerPrescaler::DIV_BY_1);
        Timer2::SetTimerMode(Timer2::TimerMode::FAST_PWM);
    }

    void Start()
    {
        running_ = 1;

        SetRed(redValCached_);
        SetGreen(greenValCached_);
        SetBlue(blueValCached_);

        Timer1::StartTimer();
        Timer2::StartTimer();
    }
    
    void Stop()
    {
        Timer1::StopTimer();
        Timer1::SetTimerValue(0);

        Timer2::StopTimer();
        Timer2::SetTimerValue(0);

        SetRGBOff();

        running_ = 0;
    }

    uint8_t GetRed()
    {
        return redValCached_;
    }
    
    uint8_t GetGreen()
    {
        return greenValCached_;
    }

    uint8_t GetBlue()
    {
        return blueValCached_;
    }

    void SetRed(uint8_t val)
    {
        SetGeneric(val, redValCached_, tcLedRed_);
    }

    void SetGreen(uint8_t val)
    {
        SetGeneric(val, greenValCached_, tcLedGreen_);
    }

    void SetBlue(uint8_t val)
    {
        SetGeneric(val, blueValCached_, tcLedBlue_);
    }

    
private:

    void SetGeneric(uint8_t val, uint8_t &cachedVal, TimerChannel *tc)
    {
        cachedVal = val;

        if (running_)
        {
            if (val == 0)
            {
                tc->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::NONE);
                tc->OutputLow();
            }
            else if (val == 255)
            {
                tc->OutputHigh();
            }
            else
            {
                tc->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
            }

            tc->SetValue(val);
        }
    }

    void SetRGBOff()
    {
        tcLedRed_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::NONE);
        tcLedRed_->OutputLow();

        tcLedGreen_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::NONE);
        tcLedGreen_->OutputLow();

        tcLedBlue_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::NONE);
        tcLedBlue_->OutputLow();
    }


private:

    TimerChannel   *tcLedRed_;
    TimerChannel   *tcLedGreen_;
    TimerChannel   *tcLedBlue_;

    uint8_t redValCached_;
    uint8_t greenValCached_;
    uint8_t blueValCached_;

    uint8_t running_;
};



#endif  // RGB_LED_PWM_CONTROLLER_H__