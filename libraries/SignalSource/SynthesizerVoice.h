#ifndef __SYNTHESIZER_VOICE_H__
#define __SYNTHESIZER_VOICE_H__


#include <util/atomic.h>

#include "Timer.h"
#include "TimedEventHandler.h"
#include "SignalOscillator.h"
#include "SignalEnvelopeADSR.h"


template <typename SignalSource, typename TimerClass>
class SynthesizerVoice
{
    //using SignalOscillatorClass = SignalOscillator<SignalSource>;
    
    static const uint16_t ENVELOPE_ATTACK_DURATION_MS  = 50;
    static const uint16_t ENVELOPE_DECAY_DURATION_MS   = 100;
    static const uint16_t ENVELOPE_SUSTAIN_LEVEL_PCT   = 60;
    static const uint16_t ENVELOPE_RELEASE_DURATION_MS = 150;
    
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
        
        // Set up envelope
        envADSR_.SetSampleRate(sampleRateActual);
        envADSR_.SetAttackDuration(ENVELOPE_ATTACK_DURATION_MS);
        envADSR_.SetDecayDuration(ENVELOPE_DECAY_DURATION_MS);
        envADSR_.SetSustainLevelPct(ENVELOPE_SUSTAIN_LEVEL_PCT);
        envADSR_.SetReleaseDuration(ENVELOPE_RELEASE_DURATION_MS);
        
        // Set up callbacks to fire when time for a sample output.
        // Code doesn't necessarily make sense here, but follows the resetting
        // of channel A above by the timer configuration.
        tca_->SetInterruptHandlerRaw(OnInterrupt);
        
        
        // Debug
        tca_->SetCTCModeBehavior(TimerChannel::CTCModeBehavior::TOGGLE);  tca_->OutputLow();
        PAL.PinMode(dbg_, OUTPUT);
    }
    
    void StartNote(uint16_t frequency, uint16_t durationMs)
    {
        // Adjust envelope
        envADSR_.Reset();
        
        // Adjust oscillator
        so_.SetFrequency(frequency);
        
        // Set timer to stop note later
        ted_.SetCallback([this](){
            StopNote();
        });
        ted_.RegisterForTimedEvent(durationMs);
        
        Serial.print("StartNote(frequency=");
        Serial.print(frequency);
        Serial.print(", durationMs=");
        Serial.print(durationMs);
        Serial.println();
    }
    
    void StopNote()
    {
        Serial.println("StopNote()");
        
        // Cancel timer in case called externally
        ted_.DeRegisterForTimedEvent();
        
        // Inform envelope that the note has been released
        envADSR_.StartDecay();
    }
    
    void Start()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Ensure valid state
            Stop();
            
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
            
            // Reset oscillator for next time
            so_.Reset();
            
            // Reset envelope for next time
            envADSR_.Reset();
        }
    }
    

private:

    static void OnInterrupt()
    {
        // Debug
        PAL.DigitalToggle(dbg_);

        // Get raw oscillator value
        int8_t oscVal = so_.GetNextSample();
        //uint8_t val = oscVal;
        
        // Get envelope value
        uint8_t envVal = envADSR_.GetNextEnvelope();
        //uint8_t val = envVal;
        
        // Do some scaling based on envelope
        int8_t scaledVal = (oscVal * envVal / 256);
        uint8_t val = 128 + scaledVal;
        
        // Create analog signal
        PORTD = val;
    }
    

    // Debug
    static Pin dbg_;
    
    
    static SignalOscillator<SignalSource> so_;
    
    static SignalEnvelopeADSR envADSR_;
    
    static constexpr TimerChannel *tca_ = TimerClass::GetTimerChannelA();
    
    static TimedEventHandlerDelegate ted_;
};


template <typename SignalSource, typename TimerClass>
Pin SynthesizerVoice<SignalSource, TimerClass>::dbg_(14, LOW);

template <typename SignalSource, typename TimerClass>
SignalOscillator<SignalSource> SynthesizerVoice<SignalSource, TimerClass>::so_;

template <typename SignalSource, typename TimerClass>
SignalEnvelopeADSR SynthesizerVoice<SignalSource, TimerClass>::envADSR_;

template <typename SignalSource, typename TimerClass>
TimedEventHandlerDelegate SynthesizerVoice<SignalSource, TimerClass>::ted_;





#endif  // __SYNTHESIZER_VOICE_H__
















