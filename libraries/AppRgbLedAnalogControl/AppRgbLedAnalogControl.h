#ifndef __APP_RGB_LED_ANALOG_CONTROL_H__
#define __APP_RGB_LED_ANALOG_CONTROL_H__


#include "Evm.h"
#include "PinInputAnalog.h"
#include "Timer1.h"
#include "Timer2.h"



struct AppRgbLedAnalogControlConfig
{
    // Red LED driven by OC1A - Pin 15
    uint8_t pinPotRed;
    
    // Green LED driven by OC1B - Pin 16
    uint8_t pinPotGreen;
    
    // Blue LED driven by OC2A - Pin 17
    uint8_t pinPotBlue;
};


class AppRgbLedAnalogControl
{
private:
    static const uint8_t C_IDLE  = PinInputAnalog::C_IDLE  * 3;
    static const uint8_t C_TIMED = PinInputAnalog::C_TIMED * 3;
    static const uint8_t C_INTER = PinInputAnalog::C_INTER * 3;
    
public:

    AppRgbLedAnalogControl(AppRgbLedAnalogControlConfig &cfg)
    : cfg_(cfg)
    , piaPotRed_(cfg_.pinPotRed)
    , tcLedRed_(Timer1::GetTimerChannelA())
    , piaPotGreen_(cfg_.pinPotGreen)
    , tcLedGreen_(Timer1::GetTimerChannelB())
    , piaPotBlue_(cfg_.pinPotBlue)
    , tcLedBlue_(Timer2::GetTimerChannelA())
    {
        // Nothing to do
    }
    
    ~AppRgbLedAnalogControl()
    {
        // Nothing to do
    }
    
    void Run()
    {
        // This app is more about learning than being an actual functional
        // standalone device.  Serial output will be used unconditionally.
        Serial.begin(9600);
        
        Serial.println("Starting AppRgbLedAnalogControl");
        
        // Set up operations
        SetUpAnalogInputs();
        SetUpLedControllingTimers();
        
        // Go
        evm_.MainLoop();
    }

private:

    void SetUpLedControllingTimers()
    {
        // Set up timers for fast wrapping, we just want high-res pwm
        Timer1::SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
        Timer1::SetTimerMode(Timer1::TimerMode::FAST_PWM_8_BIT);
        Timer1::SetTimerValue(0);
        
        Timer2::SetTimerPrescaler(Timer2::TimerPrescaler::DIV_BY_1);
        Timer2::SetTimerMode(Timer2::TimerMode::FAST_PWM);
        Timer2::SetTimerValue(0);
        
        // Set up Timer1 Channel A -- Red
        tcLedRed_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
        tcLedRed_->SetValue(0);
        
        // Set up Timer1 Channel B -- Green
        tcLedGreen_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
        tcLedGreen_->SetValue(0);
        
        // Set up Timer2 Channel A -- Blue
        tcLedBlue_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
        tcLedBlue_->SetValue(0);
        
        // Begin timer operation
        Timer1::StartTimer();
        Timer2::StartTimer();
    }
    
    void SetUpAnalogInputs()
    {
        //static const uint16_t MINIMUM_CHANGE_ANALOG_LEVEL = 5;
        static const uint16_t MINIMUM_CHANGE_ANALOG_LEVEL = 50;
        
        // Set sensitivity to analog change
        piaPotRed_.SetMinimumChange(MINIMUM_CHANGE_ANALOG_LEVEL);
        piaPotGreen_.SetMinimumChange(MINIMUM_CHANGE_ANALOG_LEVEL);
        piaPotBlue_.SetMinimumChange(MINIMUM_CHANGE_ANALOG_LEVEL);
        
        // Set up callbacks when analog inputs change
        piaPotRed_.SetCallback([this](uint16_t val){ OnPotRedChange(val); });
        piaPotGreen_.SetCallback([this](uint16_t val){ OnPotGreenChange(val); });
        piaPotBlue_.SetCallback([this](uint16_t val){ OnPotBlueChange(val); });
        
        // Enable monitoring of analog inputs
        piaPotRed_.Enable();
        piaPotGreen_.Enable();
        piaPotBlue_.Enable();
    }

    void OnPotRedChange(uint16_t val)
    {
        Serial.print("OnPotRedChange: ");
        Serial.print(val);
        Serial.println();
        
        uint8_t timerVal = (uint8_t)(val / 4);
        tcLedRed_->SetValue(timerVal);
    }
    
    void OnPotGreenChange(uint16_t val)
    {
        Serial.print("OnPotGreenChange: ");
        Serial.print(val);
        Serial.println();
        
        uint8_t timerVal = (uint8_t)(val / 4);
        tcLedGreen_->SetValue(timerVal);
    }
    
    void OnPotBlueChange(uint16_t val)
    {
        Serial.print("OnPotBlueChange: ");
        Serial.print(val);
        Serial.println();
        
        uint8_t timerVal = (uint8_t)(val / 4);
        tcLedBlue_->SetValue(timerVal);
    }


    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppRgbLedAnalogControlConfig cfg_;
    
    // Red
    PinInputAnalog  piaPotRed_;
    TimerChannel   *tcLedRed_;
    
    // Green
    PinInputAnalog  piaPotGreen_;
    TimerChannel   *tcLedGreen_;
    
    // Blue
    PinInputAnalog  piaPotBlue_;
    TimerChannel   *tcLedBlue_;
};


#endif  // __APP_RGB_LED_ANALOG_CONTROL_H__

















