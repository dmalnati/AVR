#ifndef __SYNTHESIZER_VOICE_H__
#define __SYNTHESIZER_VOICE_H__


#include <util/atomic.h>

#include "Timer.h"
#include "TimedEventHandler.h"

#include "FunctionGenerator.h"
#include "SignalEnvelopeADSR.h"

#include "CfgItem.h"


enum
{
    SET_ENVELOPE_ON_OFF              = 41,
    SET_ENVELOPE_ATTACK_DURATION_MS  = 43,
    SET_ENVELOPE_DECAY_DURATION_MS   = 44,
    SET_ENVELOPE_SUSTAIN_LEVEL_PCT   = 45,
    SET_ENVELOPE_RELEASE_DURATION_MS = 46,
};


template <typename TimerClass>
class SynthesizerVoice
: private FunctionGenerator
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
        
        SetDefaultValues();
    }
    
    ~SynthesizerVoice()
    {
        Stop();
    }

    ///////////////////////////////////////////////////////////////////////
    //
    // Initialization
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

    void EnvelopeBeginAttack()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            envADSR_.StartAttack();
        }
    }

    void EnvelopeBeginRelease()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            envADSR_.StartRelease();
        }
    }


    ///////////////////////////////////////////////////////////////////////
    //
    // Configuration
    //
    ///////////////////////////////////////////////////////////////////////

    void SetCfgItem(CfgItem c)
    {
        switch (c.type)
        {
        case SET_ENVELOPE_ON_OFF:
            SetEnvelopeOnOff((uint8_t)c);
            break;
            
        case SET_ENVELOPE_ATTACK_DURATION_MS:
            SetAttackDuration((uint16_t)c);
            break;
            
        case SET_ENVELOPE_DECAY_DURATION_MS:
            SetDecayDuration((uint16_t)c);
            break;
            
        case SET_ENVELOPE_SUSTAIN_LEVEL_PCT:
            SetSustainLevelPct((uint8_t)c);
            break;
            
        case SET_ENVELOPE_RELEASE_DURATION_MS:
            SetReleaseDuration((uint16_t)c);
            break;
            
        default:
            FunctionGenerator::SetCfgItem(c);
            break;
        }
    }
    
    void SetDefaultValues()
    {
        SetCfgItem({SET_ENVELOPE_ON_OFF, (uint8_t)1});
        
        SetCfgItem({SET_ENVELOPE_ATTACK_DURATION_MS,  ENVELOPE_ATTACK_DURATION_MS});
        SetCfgItem({SET_ENVELOPE_DECAY_DURATION_MS,   ENVELOPE_DECAY_DURATION_MS});
        SetCfgItem({SET_ENVELOPE_SUSTAIN_LEVEL_PCT,   ENVELOPE_SUSTAIN_LEVEL_PCT});
        SetCfgItem({SET_ENVELOPE_RELEASE_DURATION_MS, ENVELOPE_RELEASE_DURATION_MS});
    }

    
private:

    ///////////////////////////////////////////////////////////////////////
    //
    // EnvelopeADSR Control
    //
    ///////////////////////////////////////////////////////////////////////

    void SetEnvelopeOnOff(uint8_t onOff)
    {
        envADSREnabled_ = !!onOff;
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
uint8_t SynthesizerVoice<TimerClass>::envADSREnabled_;





#endif  // __SYNTHESIZER_VOICE_H__
















