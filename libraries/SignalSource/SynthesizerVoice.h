#ifndef __SYNTHESIZER_VOICE_H__
#define __SYNTHESIZER_VOICE_H__


#include <util/atomic.h>

#include "Timer.h"
#include "TimedEventHandler.h"

#include "SignalSourceNoneWave.h"
#include "SignalSourceSineWave.h"
#include "SignalSourceSawtoothRightWave.h"
#include "SignalSourceSawtoothLeftWave.h"
#include "SignalSourceSquareWave.h"
#include "SignalSourceTriangleWave.h"
#include "SignalOscillator.h"

#include "SignalEnvelopeADSR.h"


template <typename TimerClass>
class SynthesizerVoice
{
    static const uint16_t ENVELOPE_ATTACK_DURATION_MS  = 50;
    static const uint16_t ENVELOPE_DECAY_DURATION_MS   = 100;
    static const uint16_t ENVELOPE_SUSTAIN_LEVEL_PCT   = 60;
    static const uint16_t ENVELOPE_RELEASE_DURATION_MS = 150;
    
    static const uint16_t LFO_FREQUENCY_DEFAULT = 40;
    
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
        
        // Set up oscillators
        SetOscillator1(OscillatorType::SINE);
        SetOscillator2(OscillatorType::SINE);
        
        // Set up LFO
        SetLfo(OscillatorType::SINE);
        
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
        so1_.SetSampleRate(sampleRateActual);
        so2_.SetSampleRate(sampleRateActual);
        
        // Set up LFO
        lfo_.SetSampleRate(sampleRateActual);
        lfo_.SetFrequency(LFO_FREQUENCY_DEFAULT);
        
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
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Adjust envelope
            envADSR_.Reset();
        }
        
        // Adjust oscillator
        so1_.SetFrequency(frequency);
        so2_.SetFrequency(frequency * 2);
        
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
            so1_.Reset();
            so2_.Reset();
            
            // Reset envelope for next time
            envADSR_.Reset();
        }
    }
    
    enum class OscillatorType : uint8_t
    {
        NONE = 0,
        SINE,
        SAWR,
        SAWL,
        SQUARE,
        TRIANGLE
    };

    void SetOscillator1(OscillatorType type)
    {
        SetOscillator(so1_, type);
    }
    
    void SetOscillator2(OscillatorType type)
    {
        SetOscillator(so2_, type);
    }
    
    void SetLfo(OscillatorType type)
    {
        SetOscillator(lfo_, type);
    }

private:

    void SetOscillator(SignalOscillator &so, OscillatorType type)
    {
        if (type == OscillatorType::NONE)
        {
            static SignalSourceNoneWave ss;
            
            so.SetSignalSource(&ss.GetSample);
        }
        else if (type == OscillatorType::SINE)
        {
            static SignalSourceSineWave ss;
            
            so.SetSignalSource(&ss.GetSample);
        }
        else if (type == OscillatorType::SAWR)
        {
            static SignalSourceSawtoothRightWave ss;
            
            so.SetSignalSource(&ss.GetSample);
        }
        else if (type == OscillatorType::SAWL)
        {
            static SignalSourceSawtoothLeftWave ss;
            
            so.SetSignalSource(&ss.GetSample);
        }
        else if (type == OscillatorType::SQUARE)
        {
            static SignalSourceSquareWave ss;
            
            so.SetSignalSource(&ss.GetSample);
        }
        else if (type == OscillatorType::TRIANGLE)
        {
            static SignalSourceTriangleWave ss;
            
            so.SetSignalSource(&ss.GetSample);
        }
    }

    static void OnInterrupt()
    {
        // Debug
        PAL.DigitalToggle(dbg_);
        
        // Apply the LFO for next time
        int8_t lfoVal = lfo_.GetNextSample();
        
        int8_t freqOffset = lfoVal * lfoFactor_;
        
        so1_.ApplyFrequencyOffset(freqOffset);
        so2_.ApplyFrequencyOffset(freqOffset);

        // Get current raw oscillator value
        int8_t osc1Val = so1_.GetNextSample();
        int8_t osc2Val = so2_.GetNextSample();

        // Scale and combine oscillator values
        osc1Val = osc1Val * osc1Factor_;
        osc2Val = osc2Val * osc2Factor_;
        
        int8_t oscVal = osc1Val + osc2Val;

        // Get envelope value and apply
        Q08 envVal = envADSR_.GetNextEnvelope();

        int8_t scaledVal = (oscVal * envVal);

        // Adjust to 0-255 range
        uint8_t val = 128 + scaledVal;
        
        // Create analog signal
        PORTD = val;
    }
    

    // Debug
    static Pin dbg_;
    
    
    static SignalOscillator so1_;
    static Q08              osc1Factor_;
    
    static SignalOscillator so2_;
    static Q08              osc2Factor_;
    
    static SignalOscillator lfo_;
    static Q08              lfoFactor_;
    
    static SignalEnvelopeADSR envADSR_;
    
    static constexpr TimerChannel *tca_ = TimerClass::GetTimerChannelA();
    
    static TimedEventHandlerDelegate ted_;
};


template <typename TimerClass>
Pin SynthesizerVoice<TimerClass>::dbg_(14, LOW);

template <typename TimerClass>
SignalOscillator SynthesizerVoice<TimerClass>::so1_;
template <typename TimerClass>
Q08 SynthesizerVoice<TimerClass>::osc1Factor_ = 0.5;

template <typename TimerClass>
SignalOscillator SynthesizerVoice<TimerClass>::so2_;
template <typename TimerClass>
Q08 SynthesizerVoice<TimerClass>::osc2Factor_ = 0.5;

template <typename TimerClass>
SignalOscillator SynthesizerVoice<TimerClass>::lfo_;
template <typename TimerClass>
Q08 SynthesizerVoice<TimerClass>::lfoFactor_ = 0.5;


template <typename TimerClass>
SignalEnvelopeADSR SynthesizerVoice<TimerClass>::envADSR_;

template <typename TimerClass>
TimedEventHandlerDelegate SynthesizerVoice<TimerClass>::ted_;





#endif  // __SYNTHESIZER_VOICE_H__
















