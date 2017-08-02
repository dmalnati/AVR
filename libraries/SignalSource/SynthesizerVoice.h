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
        
        BeSingleton();
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

    // Expected to be run once, before SetSampleRate
    uint32_t GetLoopDurationUs()
    {
        FunctionGenerator::SetBenchmarkingParameters();
        
        uint32_t timeStart = PAL.Micros();
        
        const uint16_t LOOP_COUNT = 1000;
        for (uint16_t i = 0; i < LOOP_COUNT; ++i)
        {
            instance_->GetNextSample();
        }
        
        uint32_t timeEnd = PAL.Micros();
        
        FunctionGenerator::UnSetBenchmarkingParameters();
        FunctionGenerator::Reset();
        
        uint32_t timeDiff  = (timeEnd - timeStart) + 1;
        uint32_t usPerLoop = timeDiff / LOOP_COUNT;
        
        return usPerLoop;
    }
    
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
    
private:
    
    void SetDefaultValues()
    {
        SetCfgItem({SET_ENVELOPE_ON_OFF, (uint8_t)1});
    }

    void BeSingleton()
    {
        instance_ = this;
    }
    
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

    inline uint8_t GetNextSample()
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

        return val;
    }
    
    static inline void OnInterrupt()
    {
        // Output
        PORTD = instance_->GetNextSample();
    }


private:

    SignalEnvelopeADSR envADSR_;
    uint8_t            envADSREnabled_;

    static constexpr TimerChannel *tca_ = TimerClass::GetTimerChannelA();
    
    static SynthesizerVoice<TimerClass> *instance_;
};


template <typename TimerClass>
SynthesizerVoice<TimerClass> *SynthesizerVoice<TimerClass>::instance_ = NULL;



#endif  // __SYNTHESIZER_VOICE_H__
















