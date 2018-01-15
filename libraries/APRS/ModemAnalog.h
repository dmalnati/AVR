#ifndef __MODEM_ANALOG_H__
#define __MODEM_ANALOG_H__


#include "PAL.h"
#include "Timer2.h"
#include "TimerHelper.h"
#include "SignalSourceSineWave.h"
#include "SignalOscillator.h"


struct ModemAnalogFrequencyConfig
{
    SignalOscillatorFrequencyConfig fc;
    uint8_t                         preEmph;
};


class ModemAnalog
{
protected:
    static const uint16_t SAMPLE_RATE = 31250;
    
    using TimerClass = Timer2;
    using CbFnRaw    = void (*)();
    
public:

    ModemAnalog(CbFnRaw fnOnInterrupt = ModemAnalog::OnInterrupt)
    {
        fnOnInterrupt_ = fnOnInterrupt;
    }
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Initialization
    //
    ///////////////////////////////////////////////////////////////////////
    
    // Expected to be called once, at runtime, before Start or any other use.
    static void Init()
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
            th.SetInterruptFrequency(SAMPLE_RATE);
            uint16_t sampleRateActual = th.GetInterruptFrequency();
            
            // Set up Oscillator
            osc_.SetSampleRate(sampleRateActual);
            
            // Set up sample rate callback
            //
            // Code doesn't necessarily make sense here, but follows the
            // resetting of channel A above by the timer configuration.
            TimerClass::GetTimerChannelA()->SetInterruptHandlerRaw(fnOnInterrupt_);
            
            // Debug
            //TimerClass::GetTimerChannelA()->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::SPECIAL_TOP_VALUE);
        }
    }
    
    static ModemAnalogFrequencyConfig GetFrequencyConfig(uint16_t frequency,
                                                         uint8_t  preEmph = 1)
    {
        ModemAnalogFrequencyConfig fc;
        
        fc.fc      = osc_.GetFrequencyConfig(frequency);
        fc.preEmph = preEmph;
        
        return fc;
    }
    
    static void SetFrequencyByConfig(ModemAnalogFrequencyConfig *fc)
    {
        osc_.SetFrequencyByConfig(&fc->fc);
        preEmph_ = fc->preEmph;
    }
    
    static void SetFrequency(uint16_t frequency)
    {
        osc_.SetFrequency(frequency);
    }

    void Start()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Ensure valid state
            Stop();

            // Register for interrupts
            TimerClass::GetTimerChannelA()->RegisterForInterrupt();
            
            // Let sub-classes do work
            StartInternal();

            // Start the timer
            TimerClass::StartTimer();
        }
    }

    void Stop()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Stop the timer counting
            TimerClass::StopTimer();

            // Don't let any potentially queued interrupts fire
            TimerClass::GetTimerChannelA()->DeRegisterForInterrupt();
            
            // Let sub-classes do work
            StopInternal();

            // Reset oscillator for next time
            osc_.Reset();
        }
    }
    

protected:

    virtual void StartInternal()
    {
        // Nothing to do
    }
    
    virtual void StopInternal()
    {
        // Nothing to do
    }
    
    static void OnInterrupt()
    {
        // Nothing to do
    }

    static SignalSourceSineWave  ssSine_;
    static SignalOscillator      osc_;
    static uint8_t               preEmph_;
    
    static CbFnRaw  fnOnInterrupt_;

};

SignalSourceSineWave ModemAnalog::ssSine_;
SignalOscillator     ModemAnalog::osc_(&ModemAnalog::ssSine_.GetSample);
uint8_t              ModemAnalog::preEmph_(1);
ModemAnalog::CbFnRaw ModemAnalog::fnOnInterrupt_(NULL);


// Bring in child classes
#include "ModemAnalogDac.h"
#include "ModemAnalogPwm.h"


#endif  // __MODEM_ANALOG_H__










