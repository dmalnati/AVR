#ifndef __APP_ADC_2_PWM_H__
#define __APP_ADC_2_PWM_H__


#include "Timer1.h"


struct AppADC2PWMConfig
{
    // Pin to use to sense voltage
    uint8_t pinAdc;
    
    // PWM driven by OC1A - Pin 15
};

class AppADC2PWM
{
public:
    AppADC2PWM(AppADC2PWMConfig &cfg)
    : pinAdc_(cfg.pinAdc)
    , tc_(Timer1::GetTimerChannelA())
    {
        // Nothing to do
    }
    
    void Run()
    {
        Serial.begin(9600);
        Serial.println("Starting");
        
        // Set up timers for fast wrapping, we just want high-res pwm
        Timer1::SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
        Timer1::SetTimerMode(Timer1::TimerMode::FAST_PWM_8_BIT);
        Timer1::SetTimerValue(0);
        
        // Set up Timer1 Channel A for PWM output
        tc_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
        tc_->SetValue(0);
        
        // Activate PWM
        Timer1::StartTimer();

        // Main Loop
        MonitorVoltage();
    }

private:

    void MonitorVoltage()
    {
        PAL.SetADCPrescaler(PlatformAbstractionLayer::ADCPrescaler::DIV_BY_8);
        PAL.AnalogReadBatchBegin();
        
        uint8_t valScaledLast = 0;
        while (1)
        {
            uint16_t val = PAL.AnalogRead(pinAdc_);
            
            // convert from 10-bit to 8-bit
            uint8_t valScaled = (uint8_t)(val >> 2);
            
            if (valScaled != valScaledLast)
            {
                tc_->SetValue(valScaled);
                
                valScaledLast = valScaled;
            }
        }
    }

    Pin           pinAdc_;
    TimerChannel *tc_;
};



#endif  // __APP_ADC_2_PWM_H__
