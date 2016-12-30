#ifndef __SERVO_CONTROLLER_HW_H__
#define __SERVO_CONTROLLER_HW_H__


#include "PAL.h"
#include "Timer1.h"
#include "Timer2.h"


// forward decl
template <typename TimerN> class ServoControllerHWOwner;


class ServoControllerHW
{
private:
    template <typename TimerN> friend class ServoControllerHWOwner;

public:
    static const uint8_t C_IDLE  = 0;
    static const uint8_t C_TIMED = 0;
    static const uint8_t C_INTER = 0;
    
private:
    static const uint8_t PERIOD_MS           = 20;
    static const uint8_t DEFAULT_DEG_CURRENT = 90;

    // Private constructor, only allowed to be created by owner class
    ServoControllerHW(uint8_t pin, TimerChannel *tc)
    : pin_(pin)
    , tc_(tc)
    {
        // Nothing to do
    }
    
public:
    void MoveTo(uint8_t deg)
    {
        // Calculate time of pulse for given degrees
        
        // Calculate duty cycle time in microseconds.
        // Range between 0.5ms - 2.5ms to cover a 180 degree range of motion.
        uint32_t dutyCycleExpireUs = (0.5 + (((double)deg / 180.0) * 2)) * 1000.0;

        // already-known value of time per tick
        uint8_t usPerTick = 32;
        
        // convert microsecond time to a compare value
        uint8_t timerCompareVal = dutyCycleExpireUs / usPerTick;
        
        // Set timer limit and enable toggling on compare
        tc_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
        tc_->SetValue(timerCompareVal);
    }
    
    void MoveTo(uint8_t /*deg*/, uint32_t /*durationMs*/)
    {
        // Not implemented
    }
    
    void Stop()
    {
        // stop responding to timer
        tc_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::NONE);
    }


private:
    
    uint8_t       pin_;
    TimerChannel *tc_;
};


template <typename TimerN>
class ServoControllerHWOwner
{
public:
    ServoControllerHWOwner()
    : scA_(TimerN::PIN_CHANNEL_A, TimerN::GetTimerChannelA())
    , scB_(TimerN::PIN_CHANNEL_B, TimerN::GetTimerChannelB())
    {
        // Nothing to do
    }
    
    ~ServoControllerHWOwner()
    {
        TimerN::StopTimer();
        
        scA_.Stop();
        scB_.Stop();
    }
    
    void Init()
    {
        // Ideally everything would be done at construction, but the Arduino code
        // messes with some timer variables, and so waiting until runtime is
        // necessary.
        
        // Get A and B handles, disable them
        TimerChannel *channelA = TimerN::GetTimerChannelA();
        channelA->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::NONE);
        
        TimerChannel *channelB = TimerN::GetTimerChannelB();
        channelB->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::NONE);

        // Set up TimerN
        // prescaler = 1024, so full wrap time of 8-bit counter takes ~8.2ms
        TimerN::SetTimerPrescaler(TimerN::TimerPrescaler::DIV_BY_256);
        
        // mode = fastPWM
        TimerN::SetTimerModeFastPWM8Bit();

        // Can't actually set top, so instead rely on Servos caring about
        // pulse-on duration as opposed to duty cycle...
        TimerN::SetTimerValue(0);
        
        // Start Timer
        TimerN::StartTimer();
    }
    
    ServoControllerHW *GetServoControllerHWA()
    {
        return &scA_;
    }
    
    ServoControllerHW *GetServoControllerHWB()
    {
        return &scB_;
    }

    
private:

    ServoControllerHW scA_;
    ServoControllerHW scB_;
};






#endif  // __SERVO_CONTROLLER_HW_H__








