#ifndef __SIGNAL_DAC_H__
#define __SIGNAL_DAC_H__


#include <util/atomic.h>

#include "Timer.h"


//
// Clocked at 19.5us at 8MHz from pin toggle to pin toggle.
//
// That's 51,282kHz
//
// The toggle itself takes time, so the speed is actually greater if removed.
//
// Definitely starves out other interrupts at max speed, for instance
// Delay was running 3x slow.
//
template <typename SignalSource, typename TimerClass>
class SignalDAC
{
public:
    SignalDAC()
    {
        // Need better way -- open output pins
        // PORTD
        PAL.PinMode(2, OUTPUT);
        PAL.PinMode(3, OUTPUT);
        PAL.PinMode(4, OUTPUT);
        PAL.PinMode(5, OUTPUT);
        PAL.PinMode(6, OUTPUT);
        PAL.PinMode(11, OUTPUT);
        PAL.PinMode(12, OUTPUT);
        PAL.PinMode(13, OUTPUT);
        
        
        // Debug
        PAL.PinMode(dbg_, OUTPUT);
    }
    
    ~SignalDAC()
    {
        Stop();
    }
    
    void SetSampleRate(uint16_t sampleRate)
    {
        // Important - this be done during runtime, not just during static
        // init.  Arduino libs mess with timers like Timer2, and the timer
        // setup below needs to remain intact for functionality to work.

        // The timer may be running, like Timer2
        TimerClass::StopTimer();

        // Pre-calculate the actual sample rate achievable now so that
        // calls to SetFrequency will be working with the right value.
        //
        TimerHelper<TimerClass> th;
        th.SetInterruptFrequency(sampleRate);
        sampleRateActual_ = th.GetInterruptFrequency();
        
        // Set up callbacks to fire when time for a sample output.
        // Code doesn't necessarily make sense here, but follows the resetting
        // of channel A above by the timer configuration.
        tca_->SetInterruptHandlerRaw(OnInterrupt);
        
        
        // Debug
        tca_->SetCTCModeBehavior(TimerChannel::CTCModeBehavior::TOGGLE);  tca_->OutputLow();
        PAL.PinMode(dbg_, OUTPUT);
    }
    
    void SetFrequency(uint16_t frequency)
    {
        idxSignalSource_.Calibrate(sampleRateActual_, frequency);
    }
    
    void Start()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Register for interrupts
            tca_->RegisterForInterrupt();
            
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
            tca_->DeRegisterForInterrupt();
            
            // Set output value to zero
            PORTD = 0;
            
            // Adjust index to start over from beginning next time
            idxSignalSource_.ResetIdx();
        }
    }
    

private:

    static void OnInterrupt()
    {
        // Debug
        PAL.DigitalToggle(dbg_);

        
        uint8_t val = ss_.GetSampleAtIdx(idxSignalSource_);
        
        PORTD = val;
        
        ++idxSignalSource_;
    }
    

    // Debug
    static Pin dbg_;
    
    
    static uint16_t sampleRateActual_;
    
    static SignalSource           ss_;
    static typename
    SignalSource::IdxType  idxSignalSource_;
    
    static constexpr TimerChannel *tca_ = TimerClass::GetTimerChannelA();
};


template <typename SignalSource, typename TimerClass>
Pin SignalDAC<SignalSource, TimerClass>::dbg_(14, LOW);

template <typename SignalSource, typename TimerClass>
uint16_t SignalDAC<SignalSource, TimerClass>::sampleRateActual_;

template <typename SignalSource, typename TimerClass>
SignalSource SignalDAC<SignalSource, TimerClass>::ss_;

template <typename SignalSource, typename TimerClass>
typename SignalSource::IdxType SignalDAC<SignalSource, TimerClass>::idxSignalSource_;




#endif  // __SIGNAL_DAC_H__
















