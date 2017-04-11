#ifndef __APP_RGB_LED_SOUND_CONTROL_H__
#define __APP_RGB_LED_SOUND_CONTROL_H__


#include "Evm.h"
#include "IdleTimeEventHandler.h"
#include "Timer1.h"
#include "Timer2.h"
#include "SignalProcessingFHT.h"


struct AppRgbLedSoundControlConfig
{
    // Red   LED driven by OC1A - Pin 15
    // Green LED driven by OC1B - Pin 16
    // Blue  LED driven by OC2A - Pin 17
    
    // Analog input
    uint8_t pinMicAnalogInput;
};


class AppRgbLedSoundControl
{
private:
    static const uint8_t C_IDLE  = 1;   // IdleTimeEventHandlerDelegate
    static const uint8_t C_TIMED = 0;
    static const uint8_t C_INTER = 0;
    
public:

    AppRgbLedSoundControl(AppRgbLedSoundControlConfig &cfg)
    : cfg_(cfg)
    , pinMicAnalogInput_(cfg_.pinMicAnalogInput)
    , tcLedRed_(Timer1::GetTimerChannelA())
    , tcLedGreen_(Timer1::GetTimerChannelB())
    , tcLedBlue_(Timer2::GetTimerChannelA())

    {
        // Nothing to do
    }
    
    ~AppRgbLedSoundControl()
    {
        // Nothing to do
    }
    
    void Run()
    {
        // Set up operations
        SetUpLedControllingTimers();
        SetUpAnalogInputs();
        
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
        timerAdcFht_.SetCallback([this](){
            OnDoAdcFht();
        });
        timerAdcFht_.RegisterForIdleTimeEvent();
    }
    
    void OnDoAdcFht()
    {
        SignalProcessingFHT::Measurement m;

        if (fht_.GetMeasurement(pinMicAnalogInput_, &m))
        {
            OnFhtData(m.valList);
        }
    }
    
    void OnFhtData(uint16_t *fhtValList)
    {
        uint8_t valRed   = CalculateRange(fhtValList,  2, 16);
        uint8_t valBlue  = CalculateRange(fhtValList,  8, 20);
        uint8_t valGreen = CalculateRange(fhtValList, 12, 24);
        
        OnNewValue(valRed,   tcLedRed_,   1);
        OnNewValue(valBlue,  tcLedBlue_,  2);
        OnNewValue(valGreen, tcLedGreen_, 4);
    }
    
    void OnNewValue(uint8_t val, TimerChannel *tc, uint8_t stepDown)
    {
        int16_t curValBig  = (int16_t)tc->GetValue();
        int16_t thisValBig = val;
        
        if (thisValBig < curValBig)
        {
            // Decrement by stepDown unless that would overshoot, at which
            // point simply go as far as you can.
            uint16_t stepDownVal = stepDown;
            if ((curValBig - stepDown) < thisValBig)
            {
                stepDownVal = curValBig - thisValBig;
            }
            
            curValBig -= stepDownVal;
            
            // Set channel value
            tc->SetValue(curValBig);
        }
        else
        {
            // Set immediately, higher values always are immediate
            
            // Set channel value
            tc->SetValue(thisValBig);
        }
    }
    
    uint8_t CalculateRange(uint16_t *fhtValList,
                           uint16_t  idxStart,
                           uint16_t  idxLim)
    {
        static const uint8_t MIN_VALUE =  10;
        static const uint8_t MAX_VALUE = 255;
        
        uint16_t count = 0;
        for (uint16_t i = idxStart; i < idxLim; ++i)
        {
            if (MIN_VALUE < fhtValList[i])
            {
                count += fhtValList[i];
            }
        }
        
        uint8_t retVal = (count <= MAX_VALUE) ? count : MAX_VALUE;
        
        return retVal;
    }
    


    
    // Event manager
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    // Config
    AppRgbLedSoundControlConfig cfg_;
    
    // Signal processor
    SignalProcessingFHT fht_;
    
    // Analog input
    Pin pinMicAnalogInput_;
    
    // Timer channels for each color
    TimerChannel *tcLedRed_;
    TimerChannel *tcLedGreen_;
    TimerChannel *tcLedBlue_;
    
    // Some time keeping
    IdleTimeEventHandlerDelegate timerAdcFht_;
};


#endif  // __APP_RGB_LED_SOUND_CONTROL_H__

















