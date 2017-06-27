#ifndef __SIGNAL_DAC_H__
#define __SIGNAL_DAC_H__


#include <util/atomic.h>

#include "Timer2.h"



/*
 * Callers operate the frequency to be generated, not the sampling rate at
 * which it is created.
 */


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
    }
    
    ~SignalDAC()
    {
        Stop();
    }
    
    void SetSampleRate(uint16_t sampleRate)
    {
        // Pre-calculate the actual sample rate achievable now so that
        // calls to SetFrequency will be working with the right value.
        //
        // We'll re-calculate again later in case something happens between
        // static init and runtime (as is likely with timers and Arduino)
        TimerHelper<TimerClass> th;
        th.SetInterruptFrequency(sampleRate);
        sampleRateActual_ = th.GetInterruptFrequency();
    }
    
    void SetFrequency(uint16_t frequency)
    {
        idxSignalSource_.Calibrate(sampleRateActual_, frequency);
    }
    
    void Start()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // The timer may be running, like Timer2
            TimerClass::StopTimer();
            
            // Configure to operate at given sample rate.
            // This was done earlier but must be re-done for safety.
            TimerHelper<TimerClass>{}.SetInterruptFrequency(sampleRateActual_);
            
            // Set up handling callbacks at the configured interval
            PAL.PinMode(dbg_, OUTPUT);
            tca_->SetInterruptHandler([this](){
                //PAL.DigitalWrite(dbg_, HIGH);
                PAL.DigitalToggle(dbg_);
                OnInterrupt();
                //PAL.DigitalWrite(dbg_, LOW);
            });
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
        }
    }
    

private:

    void OnInterrupt()
    {
        uint8_t val = ss_.GetSampleAtIdx(idxSignalSource_);
        
        PORTD = val;
        
        ++idxSignalSource_;
    }
    
    
    
    Pin dbg_;
    

    uint16_t sampleRateActual_;
    
    SignalSource           ss_;
    typename
    SignalSource::IdxType  idxSignalSource_;
    
    //constexpr
    TimerChannel *tca_ = TimerClass::GetTimerChannelA();
};

/*
template <typename SignalSource>
SignalSource SignalDAC<SignalSource>::ss_;
*/

#endif  // __SIGNAL_DAC_H__
















