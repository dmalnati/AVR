#ifndef __SYNTHESIZER_VOICE_H__
#define __SYNTHESIZER_VOICE_H__


#include <util/atomic.h>

#include "Timer.h"
#include "SignalOscillator.h"


template <typename SignalSource, typename TimerClass>
class SynthesizerVoice
{
    //using SignalOscillatorClass = SignalOscillator<SignalSource>;
    
public:
    SynthesizerVoice()
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
    
    ~SynthesizerVoice()
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
        uint16_t sampleRateActual = th.GetInterruptFrequency();
        
        // Set up oscillator
        so_.SetSampleRate(sampleRateActual);
        
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
        so_.SetFrequency(frequency);
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
            so_.Reset();
        }
    }
    

private:

    static void OnInterrupt()
    {
        // Debug
        PAL.DigitalToggle(dbg_);

        
        uint8_t val = so_.GetNextSample();
        
        PORTD = val;
    }
    

    // Debug
    static Pin dbg_;
    
    
    static SignalOscillator<SignalSource> so_;
    
    static constexpr TimerChannel *tca_ = TimerClass::GetTimerChannelA();
};


template <typename SignalSource, typename TimerClass>
Pin SynthesizerVoice<SignalSource, TimerClass>::dbg_(14, LOW);

template <typename SignalSource, typename TimerClass>
SignalOscillator<SignalSource> SynthesizerVoice<SignalSource, TimerClass>::so_;




#endif  // __SYNTHESIZER_VOICE_H__
















