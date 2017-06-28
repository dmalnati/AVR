#ifndef __SIGNAL_DAC_H__
#define __SIGNAL_DAC_H__


#include <util/atomic.h>

#include "Timer2.h"


template <typename SignalSource>
class SignalDAC
{
    using TimerClass = Timer2;
    
public:
    SignalDAC()
    : dbg_(14)
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
        tca_->SetInterruptHandler([this](){
            //PAL.DigitalWrite(dbg_, HIGH);
            
            PAL.DigitalToggle(dbg_);
            
            OnInterrupt();
            
            //PAL.DigitalWrite(dbg_, LOW);
        });
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

    void OnInterrupt()
    {
        uint8_t val = ss_.GetSampleAtIdx(idxSignalSource_);
        
        PORTD = val;
        
        ++idxSignalSource_;
    }
    

    // Debug
    Pin dbg_;
    
    
    uint16_t sampleRateActual_;
    
    SignalSource           ss_;
    typename
    SignalSource::IdxType  idxSignalSource_;
    
    TimerChannel *tca_ = TimerClass::GetTimerChannelA();
};



#endif  // __SIGNAL_DAC_H__
















