#ifndef __MODEM_ANALOG_PWM_H__
#define __MODEM_ANALOG_PWM_H__


#include "PAL.h"
#include "Timer1.h"
#include "Timer2.h"
#include "TimerHelper.h"
#include "SignalSourceSineWave.h"
#include "SignalOscillator.h"


struct ModemAnalogPwmFrequencyConfig
{
    SignalOscillatorFrequencyConfig fc;
    uint8_t                         preEmph;
};


class ModemAnalogPwm
{
    using TimerClass = Timer2;
    
public:

    ModemAnalogPwm()
    {
        // Nothing to do
    }
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Initialization
    //
    ///////////////////////////////////////////////////////////////////////
    
    // Expected to be called once, at runtime, before Start or any other use.
    static void SetSampleRate(uint16_t sampleRate)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Important - this be done during runtime, not just during static
            // init.  Arduino libs mess with timers like Timer2, and the timer
            // setup below needs to remain intact for functionality to work.

            // The timer may be running on system startup, like Timer2
            TimerClass::StopTimer();

            // Pre-calculate the actual sample rate achievable now so that
            // calls to SetFrequency will be working with the right value.
            //
            TimerHelper<TimerClass> th;
            th.SetInterruptFrequency(sampleRate);
            uint16_t sampleRateActual = th.GetInterruptFrequency();
            
            // Set up Oscillator
            osc_.SetSampleRate(sampleRateActual);
            
            // Set up sample rate callback
            //
            // Code doesn't necessarily make sense here, but follows the
            // resetting of channel A above by the timer configuration.
            TimerClass::GetTimerChannelA()->SetInterruptHandlerRaw(OnInterrupt);
            
            // Debug
            //TimerClass::GetTimerChannelA()->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::SPECIAL_TOP_VALUE);
        
        
            TimerChannel *tcB = TimerClass::GetTimerChannelB();
            
            tcB->DeRegisterForInterrupt();
            tcB->UnSetInterruptHandler();
            //tcB->SetCTCModeBehavior(TimerChannel::CTCModeBehavior::CLEAR);
            tcB->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
        }
    }
    
    static ModemAnalogPwmFrequencyConfig GetFrequencyConfig(uint16_t frequency,
                                                            uint8_t  preEmph = 1)
    {
        ModemAnalogPwmFrequencyConfig fc;
        
        fc.fc      = osc_.GetFrequencyConfig(frequency);
        fc.preEmph = preEmph;
        
        return fc;
    }
    
    static void SetFrequencyByConfig(ModemAnalogPwmFrequencyConfig *fc)
    {
        osc_.SetFrequencyByConfig(&fc->fc);
        preEmph_ = fc->preEmph;
    }
    
    static void SetFrequency(uint16_t frequency)
    {
        osc_.SetFrequency(frequency);
    }

    static void Start()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Ensure valid state
            Stop();

            // Register for interrupts
            TimerClass::GetTimerChannelA()->RegisterForInterrupt();

            // Start the timer
            TimerClass::StartTimer();
            
            
            
            
            
            /*
            Timer1::SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
            Timer1::SetTimerMode(Timer1::TimerMode::FAST_PWM_8_BIT);
            Timer1::SetTimerValue(0);
            
            // Set up Timer1 Channel A for PWM output
            Timer1::GetTimerChannelA()->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
            Timer1::GetTimerChannelA()->SetValue(0);
            
            // Activate PWM
            Timer1::StartTimer();
            */
        }
    }

    static void Stop()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Stop the timer counting
            TimerClass::StopTimer();

            // Don't let any potentially queued interrupts fire
            TimerClass::GetTimerChannelA()->DeRegisterForInterrupt();

            // Reset oscillator for next time
            osc_.Reset();
        }
    }
    

private:

    
    static inline void OnInterrupt()
    {
        // Adjust to 0-255 range
        //uint8_t val = 128 + (osc_.GetNextSample() / preEmph_);
//        uint8_t val = 128 + osc_.GetNextSample();
        
        

        //uint8_t val = 128 + osc_.GetNextSample();   // works
        uint8_t val = 128 + (osc_.GetNextSample() >> 1);
        //uint8_t val = 128 + (preEmph_ != 1 ? osc_.GetNextSample() >> 2 : osc_.GetNextSample());
        TimerClass::GetTimerChannelB()->SetValue(val);
        
        
        //uint16_t val2 = val * TimerClass::GetTimerChannelA()->GetValue() / 256;
        
        //TimerClass::GetTimerChannelB()->SetValue(val2);
    }
    

    static SignalSourceSineWave  ssSine_;
    static SignalOscillator      osc_;
    static uint8_t               preEmph_;

};

SignalSourceSineWave ModemAnalogPwm::ssSine_;
SignalOscillator     ModemAnalogPwm::osc_(&ModemAnalogPwm::ssSine_.GetSample);
uint8_t              ModemAnalogPwm::preEmph_(1);


#endif  // __MODEM_ANALOG_PWM_H__










