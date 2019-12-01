#ifndef __APP_JAGGER_TOY1_H__
#define __APP_JAGGER_TOY1_H__


#include "PAL.h"
#include "Log.h"
#include "Timer1.h"
#include "Evm.h"
#include "PinInputAnalog.h"
#include "ServoControllerHW.h"


struct AppJaggerToy1Config
{
    uint8_t pinInputSpeed  = 23;    // analog in
    uint8_t pinOutputSpeed = 15;    // genuine PWM Timer1 A

    uint8_t pinInputRotate1 = 24;   // analog in, pin 17 Timer2 A PWM out
    uint8_t pinInputRotate2 = 25;   // analog in, pin  5 Timer2 B PWM out
};


class AppJaggerToy1
{
    
public:

    AppJaggerToy1(AppJaggerToy1Config &cfg)
    : cfg_(cfg)
    , inputSpeed_(cfg_.pinInputSpeed, 10)
    , tcSpeed_(Timer1::GetTimerChannelA())
    , inputRotate1_(cfg_.pinInputRotate1)
    , inputRotate2_(cfg_.pinInputRotate2)
    , servoRotate1_(servoOwner_.GetServoControllerHWA())
    , servoRotate2_(servoOwner_.GetServoControllerHWB())
    {
        // Nothing to do
    }
    
    void Run()
    {
        LogStart(9600);
        Log("Starting");

        // Set up physical interface
        SetupPhysicalInterface();
        
        // Handle events
        Log("Running");
        evm_.MainLoop();
    }


private:

    void SetupPhysicalInterface()
    {
        inputSpeed_.SetCallback([this](uint16_t val){
            OnSpeedChange(val);
        });
        inputSpeed_.SetMinimumChange(5);
        inputSpeed_.Enable();

        // Set up timers for fast wrapping, we just want high-res pwm
        Timer1::SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_64);
        Timer1::SetTimerMode(Timer1::TimerMode::FAST_PWM_8_BIT);
        Timer1::SetTimerValue(0);

        // Set up Timer1 Channel A -- Red
        tcSpeed_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
        tcSpeed_->SetValue(0);
        
        Timer1::StartTimer();

        inputRotate1_.SetCallback([this](uint16_t val){
            OnRotate1Change(val);
        });
        inputRotate1_.Enable();

        inputRotate2_.SetCallback([this](uint16_t val){
            OnRotate2Change(val);
        });
        inputRotate2_.Enable();

        servoOwner_.Init();
    }
    
    void OnSpeedChange(uint16_t val)
    {
        Log("Speed changed to ", val);

        // convert into a top speed which is safe for jagger.
        // From empirical tests, at 5v, this is ~60/255.
        // So scale to that value from the 10-bit ADC reading.
        uint8_t pwmVal = (uint32_t)val * 60 / 1024;

        tcSpeed_->SetValue(pwmVal);
        Log("PWM changed to ", pwmVal);
    }

    void OnRotate1Change(uint16_t val)
    {
        Log("Rotate1 changed to ", val);

        // convert to 180 degree movement
        uint16_t moveTo = (uint32_t)val * 180 / 1024;
        Log("Moving servo1 to: ", moveTo);
        servoRotate1_.MoveTo(moveTo);
    }

    void OnRotate2Change(uint16_t val)
    {
        Log("Rotate2 changed to ", val);

        // convert to 180 degree movement
        uint16_t moveTo = (uint32_t)val * 180 / 1024;
        Log("Moving servo2 to: ", moveTo);
        servoRotate2_.MoveTo(moveTo);
    }



private:

    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppJaggerToy1Config &cfg_;
    
    PinInputAnalog inputSpeed_;
    TimerChannel *tcSpeed_;

    PinInputAnalog inputRotate1_;
    PinInputAnalog inputRotate2_;

    ServoControllerHWOwner2  servoOwner_;
    ServoControllerHW       &servoRotate1_;
    ServoControllerHW       &servoRotate2_;
};



































#endif  // __APP_JAGGER_TOY1_H__


