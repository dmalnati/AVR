#ifndef __SYNTHESIZER_VOICE_H__
#define __SYNTHESIZER_VOICE_H__


#include <util/atomic.h>

#include "Timer.h"
#include "TimedEventHandler.h"

#include "FunctionGenerator.h"
#include "SignalEnvelopeADSR.h"


template <typename TimerClass>
class SynthesizerVoice
: public FunctionGenerator
{
    static const uint16_t ENVELOPE_ATTACK_DURATION_MS  = 50;
    static const uint16_t ENVELOPE_DECAY_DURATION_MS   = 100;
    static const uint16_t ENVELOPE_SUSTAIN_LEVEL_PCT   = 60;
    static const uint16_t ENVELOPE_RELEASE_DURATION_MS = 150;
    
public:

    SynthesizerVoice()
    {
        // Need better way -- open output pins
        // PORTD
        //PAL.PinMode(2, OUTPUT);
        PAL.PinMode(3, OUTPUT);
        PAL.PinMode(4, OUTPUT);
        PAL.PinMode(5, OUTPUT);
        PAL.PinMode(6, OUTPUT);
        PAL.PinMode(11, OUTPUT);
        PAL.PinMode(12, OUTPUT);
        PAL.PinMode(13, OUTPUT);
    }
    
    ~SynthesizerVoice()
    {
        Stop();
    }
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Primary Initialization
    //
    ///////////////////////////////////////////////////////////////////////
    
    // Expected to be called once, at runtime, before Start or any other use.
    void SetSampleRate(uint16_t sampleRate)
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
            
            // Set up Function Generator
            FunctionGenerator::SetSampleRate(sampleRateActual);
            
            // Set up Envelope
            envADSR_.SetSampleRate(sampleRateActual);
            envADSR_.SetAttackDuration(ENVELOPE_ATTACK_DURATION_MS);
            envADSR_.SetDecayDuration(ENVELOPE_DECAY_DURATION_MS);
            envADSR_.SetSustainLevelPct(ENVELOPE_SUSTAIN_LEVEL_PCT);
            envADSR_.SetReleaseDuration(ENVELOPE_RELEASE_DURATION_MS);
            
            // Set up sample rate callback
            //
            // Code doesn't necessarily make sense here, but follows the
            // resetting of channel A above by the timer configuration.
            tca_->SetInterruptHandlerRaw(OnInterrupt);
            
            
            // Debug
            tca_->SetCTCModeBehavior(TimerChannel::CTCModeBehavior::TOGGLE);
            tca_->OutputLow();
        }
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
            
            // Reset oscillators for next time
            FunctionGenerator::Reset();
            
            // Reset envelope for next time
            envADSR_.Reset();
        }
    }
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Music Synthesis Interface
    //
    ///////////////////////////////////////////////////////////////////////
    
    void OnKeyDown()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            envADSR_.Reset();
        }
    }
    
    void OnKeyUp()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            envADSR_.StartDecay();
        }
    }

    
    
    ///////////////////////////////////////////////////////////////////////
    //
    // EnvelopeADSR Control
    //
    ///////////////////////////////////////////////////////////////////////

    void EnableEnvelopeADSR()
    {
        envADSREnabled_ = 1;
    }
    
    void DisableEnvelopeADSR()
    {
        envADSREnabled_ = 0;
    }
    
    void SetAttackDuration(uint16_t durationMs)
    {
        envADSR_.SetAttackDuration(durationMs);
    }
    
    void SetDecayDuration(uint16_t durationMs)
    {
        envADSR_.SetDecayDuration(durationMs);
    }
    
    void SetSustainLevelPct(uint8_t levelPct)
    {
        envADSR_.SetSustainLevelPct(levelPct);
    }
    
    void SetReleaseDuration(uint16_t durationMs)
    {
        envADSR_.SetReleaseDuration(durationMs);
    }
    
private:
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Main Synthesis Loop
    //
    ///////////////////////////////////////////////////////////////////////

    static void OnInterrupt()
    {
        // Get next generated value
        int8_t fgVal = FunctionGenerator::GetNextValue();
        
        // Get envelope value and apply
        Q08 envVal = envADSR_.GetNextEnvelope();

        int8_t scaledVal = fgVal;
        if (envADSREnabled_)
        {
            scaledVal = (fgVal * envVal);
        }
        
        // Adjust to 0-255 range
        uint8_t val = 128 + scaledVal;

        // Output
        PORTD = val;
    }
    
    
    
private:
    
    static SignalEnvelopeADSR envADSR_;
    static uint8_t            envADSREnabled_;
    
    static constexpr TimerChannel *tca_ = TimerClass::GetTimerChannelA();
};



template <typename TimerClass>
SignalEnvelopeADSR SynthesizerVoice<TimerClass>::envADSR_;
template <typename TimerClass>
uint8_t SynthesizerVoice<TimerClass>::envADSREnabled_ = 1;





#endif  // __SYNTHESIZER_VOICE_H__
















