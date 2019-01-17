#ifndef __APP_IR_TO_PWM_H__
#define __APP_IR_TO_PWM_H__


#include "Log.h"
#include "Evm.h"
#include "Timer1.h"
#include "SensorIR_Remote.h"


struct AppIRtoPWMConfig
{
    uint8_t pinIrSensor;
    
    // PWM on OC1A - Pin 15
};


class AppIRtoPWM
{
private:
    
    static const uint8_t  INTENSITY_STEP_SIZE = 255 / 9;
    static const uint32_t TWEEN_DURATION_MS   = 1000;
    
    
public:
    AppIRtoPWM(AppIRtoPWMConfig &cfg)
    : cfg_(cfg)
    , irSensor_(cfg_.pinIrSensor)
    , tcPwm_(Timer1::GetTimerChannelA())
    , intensity_(0)
    , buttonNum_(0)
    , animationRunning_(0)
    {
        // Nothing to do
    }
    
    void Run()
    {
        LogStart(9600);
        Log("Starting");
        
        SetupPWM();
        SetupIR();
        
        Log("Running");
        evm_.MainLoop();
    }

private:


//////////////////////////////////////////////////////////////////////
//
// Setup
//
//////////////////////////////////////////////////////////////////////

    void SetupPWM()
    {
        DisablePWM();
        
        SetIntensity(0);
    }

    void EnablePWM()
    {
        // Set up timers for fast wrapping, we just want high-res pwm
        Timer1::SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
        Timer1::SetTimerMode(Timer1::TimerMode::FAST_PWM_8_BIT);
        Timer1::SetTimerValue(0);
        
        // Set up Timer1 Channel A
        tcPwm_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
        tcPwm_->SetValue(0);
        
        // Make sure timer is not running
        Timer1::StartTimer();
    }
    
    void DisablePWM()
    {
        tcPwm_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::NONE);
        Timer1::StopTimer();
    }
    
    void SetupIR()
    {
        irSensor_.SetCallback([&](SensorIR_Remote::Button b){
            OnButton(b);
        });
        
        irSensor_.Start();
    }
    
    
//////////////////////////////////////////////////////////////////////
//
// User Async events
//
//////////////////////////////////////////////////////////////////////

    void OnButton(SensorIR_Remote::Button b)
    {
        if      (b == SensorIR_Remote::Button::POWER)     { (IsOn() ? TurnOff() : TurnOn()); }
        else if (b == SensorIR_Remote::Button::CH_PLUS)   { IncrIntensity();  }
        else if (b == SensorIR_Remote::Button::CH_MINUS)  { DecrIntensity();  }
        else if (b == SensorIR_Remote::Button::VOL_PLUS)  { IncrIntensity();  }
        else if (b == SensorIR_Remote::Button::VOL_MINUS) { DecrIntensity();  }
        else if (b == SensorIR_Remote::Button::NUM_0)     { OnButtonNum(0);  }
        else if (b == SensorIR_Remote::Button::NUM_1)     { OnButtonNum(1);  }
        else if (b == SensorIR_Remote::Button::NUM_2)     { OnButtonNum(2);  }
        else if (b == SensorIR_Remote::Button::NUM_3)     { OnButtonNum(3);  }
        else if (b == SensorIR_Remote::Button::NUM_4)     { OnButtonNum(4);  }
        else if (b == SensorIR_Remote::Button::NUM_5)     { OnButtonNum(5);  }
        else if (b == SensorIR_Remote::Button::NUM_6)     { OnButtonNum(6);  }
        else if (b == SensorIR_Remote::Button::NUM_7)     { OnButtonNum(7);  }
        else if (b == SensorIR_Remote::Button::NUM_8)     { OnButtonNum(8);  }
        else if (b == SensorIR_Remote::Button::NUM_9)     { OnButtonNum(9);  }
    }

    
//////////////////////////////////////////////////////////////////////
//
// Button Number Interface
//
//////////////////////////////////////////////////////////////////////

    void OnButtonNum(uint8_t buttonNum)
    {
        buttonNum_ = buttonNum;
        
        SetIntensityAsync(ButtonNumToIntensity(buttonNum_));
    }
    
    uint8_t FadeIsRunning()
    {
        return animationRunning_;
    }
    
    void TurnOn()
    {
        if (!FadeIsRunning()) { OnButtonNum(9); }
    }
    
    void TurnOff()
    {
        if (!FadeIsRunning()) { OnButtonNum(0); }

    }
    
    void IncrIntensity()
    {
        if (buttonNum_ < 9)
        {
            if (!FadeIsRunning()) { OnButtonNum(buttonNum_ + 1); }
        }
    }

    void DecrIntensity()
    {
        if (buttonNum_ > 0)
        {
            if (!FadeIsRunning()) { OnButtonNum(buttonNum_ - 1); }
        }
    }

    
//////////////////////////////////////////////////////////////////////
//
// Intensity Interface
//
//////////////////////////////////////////////////////////////////////

    void SetIntensityAsync(uint8_t val)
    {
        uint8_t intensityOld = intensity_;
        uint8_t intensityNew = val;
        
        uint8_t intensityDiff;
        int8_t  stepDir;
        
        
        if (intensityOld < intensityNew)
        {
            stepDir = 1;
            
            intensityDiff = intensityNew - intensityOld;
        }
        else
        {
            stepDir = -1;
            
            intensityDiff = intensityOld - intensityNew;
        }
        
        if (intensityDiff)
        {
            uint32_t stepIntervalMs = TWEEN_DURATION_MS / intensityDiff;
            
            ted_.SetCallback([=](){
                if (intensity_ != intensityNew)
                {
                    if (stepDir == 1)
                    {
                        SetIntensity(intensity_ + 1);
                    }
                    else
                    {
                        SetIntensity(intensity_ - 1);
                    }
                }
                else
                {
                    ted_.DeRegisterForTimedEvent();
                    
                    buttonNum_ = intensity_ / INTENSITY_STEP_SIZE;
                    
                    animationRunning_ = 0;
                }
            });
            
            ted_.RegisterForTimedEventInterval(stepIntervalMs, 0);
            
            animationRunning_ = 1;
        }
    }
    
    void SetIntensity(uint8_t val)
    {
        if (val == 0 || val == 255)
        {
            DisablePWM();
            
            if (val == 0)
            {
                tcPwm_->OutputLow();
            }
            else
            {
                tcPwm_->OutputHigh();
            }
        }
        else
        {
            EnablePWM();
            
            tcPwm_->SetValue(val);
        }
        
        intensity_ = val;
    }
    
    
//////////////////////////////////////////////////////////////////////
//
// State Interface
//
//////////////////////////////////////////////////////////////////////

    uint8_t IsOn()
    {
        return intensity_;
    }
    
    uint8_t ButtonNumToIntensity(uint8_t stepCount)
    {
        return (uint8_t)(stepCount * INTENSITY_STEP_SIZE);
    }
    


private:

    static const uint8_t C_IDLE  = 0;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 1;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppIRtoPWMConfig &cfg_;
    
    SensorIR_Remote irSensor_;
    
    TimerChannel *tcPwm_;
    
    uint8_t intensity_;
    uint8_t buttonNum_;
    
    TimedEventHandlerDelegate ted_;
    uint8_t animationRunning_;
};




#endif  // __APP_IR_TO_PWM_H__



