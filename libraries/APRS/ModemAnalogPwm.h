#ifndef __MODEM_ANALOG_PWM_H__
#define __MODEM_ANALOG_PWM_H__


#include "ModemAnalog.h"


class ModemAnalogPwm
: public ModemAnalog
{
public:

    ModemAnalogPwm()
    : ModemAnalog(OnInterrupt)
    {
        // Nothing to do
    }
    
private:

    virtual void StartInternal()
    {
        TimerChannel *tcB = TimerClass::GetTimerChannelB();
        
        tcB->DeRegisterForInterrupt();
        tcB->UnSetInterruptHandler();
        tcB->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
        tcB->SetValue(128);
    }
    
    static inline void OnInterrupt()
    {
        // Adjust to 0-255 range
        uint8_t val = 128 + (osc_.GetNextSample() >> preEmph_);
        TimerClass::GetTimerChannelB()->SetValue(val);
    }
};


#endif  // __MODEM_ANALOG_PWM_H__










