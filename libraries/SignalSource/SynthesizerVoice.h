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

    enum class OscillatorType : uint8_t
    {
        NONE = 0,
        SINE,
        SAWR,
        SAWL,
        SQUARE,
        TRIANGLE
    };

    
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
        
        // Set up oscillators
        SetOscillator1WaveType(OscillatorType::SINE);
        SetOscillator2WaveType(OscillatorType::NONE);
        
        // Set up LFO
        SetLFOWaveType(OscillatorType::SINE);
        
        // Debug
        PAL.PinMode(dbg_, OUTPUT);
    }
    
    ~SynthesizerVoice()
    {
        Stop();
    }
    
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
            
            // Set up oscillator
            osc1_.SetSampleRate(sampleRateActual);
            osc2_.SetSampleRate(sampleRateActual);
            
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
    }
    
    void StartNote(uint16_t frequency, uint16_t durationMs)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Adjust envelope
            envADSR_.Reset();
        
            // Adjust oscillator
            osc1_.SetFrequency(frequency);
            osc2_.SetFrequency(frequency * 2);
        }
        
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
            
            // Reset oscillators for next time
            SyncAllOscillators();
            
            // Reset envelope for next time
            envADSR_.Reset();
        }
    }
    
    
    
    
    void SetPhaseLock(uint8_t phaseLock)
    {
        phaseLock_ = !!phaseLock;
        
        if (phaseLock_)
        {
            SyncAllOscillators();
        }
    }
    
    
    
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Oscillator 1
    //
    ///////////////////////////////////////////////////////////////////////
    
    void SetOscillator1WaveType(OscillatorType type)
    {
        SetOscillator(osc1_, osc1Enabled_, type);
    }
    
    void SetOscillator1Frequency(uint16_t frequency)
    {
        osc1_.SetFrequency(frequency);
        
        if (phaseLock_)
        {
            SyncAllOscillators();
        }
    }
    
    void SetOscillator1PhaseOffset(int8_t offset)
    {
        osc1_.SetPhaseOffset(offset);
    }
    
    
    
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Oscillator 2
    //
    ///////////////////////////////////////////////////////////////////////

    void SetOscillator2WaveType(OscillatorType type)
    {
        SetOscillator(osc2_, osc2Enabled_, type);
    }
    
    void SetOscillator2Frequency(uint16_t frequency)
    {
        osc2_.SetFrequency(frequency);
        
        if (phaseLock_)
        {
            SyncAllOscillators();
        }
    }
    
    void SetOscillator2PhaseOffset(int8_t offset)
    {
        osc2_.SetPhaseOffset(offset);
    }
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Oscillator Balance
    //
    ///////////////////////////////////////////////////////////////////////
    
    void SetOscillatorBalance(uint8_t balance)
    {
        // Input ranges from 0 to 255
        // 127 and 128 = 50% for each
        //   0 = 100% osc1
        // 255 = 100% osc2
        
        uint8_t osc1Pct;
        uint8_t osc2Pct;
        
        if (balance == 127 || balance == 128)
        {
            osc1Pct = 127;
            osc2Pct = 127;
        }
        else if (balance > 128)
        {
            osc2Pct = 127 + ((balance - 128) + 1);
            osc1Pct = 255 - osc2Pct;
        }
        else
        {
            osc1Pct = 127 + ((127 - balance) + 1);
            osc2Pct = 255 - osc1Pct;
        }
        
        osc1Factor_ = osc1Pct;
        osc2Factor_ = osc2Pct;
    }
    
    ///////////////////////////////////////////////////////////////////////
    //
    // LFO
    //
    ///////////////////////////////////////////////////////////////////////
    
    void SetLFOWaveType(OscillatorType type)
    {
        SetOscillator(lfo_, lfoEnabled_, type);
        
        if (!lfoEnabled_)
        {
            // Restore the frequency offset to the other oscillators
            osc1_.ApplyFrequencyOffsetPctIncreaseFromBase((uint8_t)0);
            osc2_.ApplyFrequencyOffsetPctIncreaseFromBase((uint8_t)0);
        }
    }
    
    void SetLFOFrequency(uint16_t frequency)
    {
        lfo_.SetFrequency(frequency);
        
        if (phaseLock_)
        {
            SyncAllOscillators();
        }
    }
    
    void SetLFOPhaseOffset(int8_t offset)
    {
        lfo_.SetPhaseOffset(offset);
    }
    
    void SetLFOVibratoPct(uint8_t vibratoPct)
    {
        lfoVibratoPct_ = vibratoPct;
    }
    

    ///////////////////////////////////////////////////////////////////////
    //
    // EnvelopeADSR
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
    
private:

    ///////////////////////////////////////////////////////////////////////
    //
    // Oscillator Control
    //
    ///////////////////////////////////////////////////////////////////////
    
    void SyncAllOscillators()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            osc1_.Reset();
            osc2_.Reset();
            lfo_.Reset();
        }
    }

    void SetOscillator(SignalOscillator &osc,
                       uint8_t          &oscEnabled,
                       OscillatorType    type)
    {
        oscEnabled = 1;
        
        if (type == OscillatorType::NONE)
        {
            static SignalSourceNoneWave ss;
            
            oscEnabled = 0;
            
            osc.SetSignalSource(&ss.GetSample);
        }
        else if (type == OscillatorType::SINE)
        {
            static SignalSourceSineWave ss;
            
            osc.SetSignalSource(&ss.GetSample);
        }
        else if (type == OscillatorType::SAWR)
        {
            static SignalSourceSawtoothRightWave ss;
            
            osc.SetSignalSource(&ss.GetSample);
        }
        else if (type == OscillatorType::SAWL)
        {
            static SignalSourceSawtoothLeftWave ss;
            
            osc.SetSignalSource(&ss.GetSample);
        }
        else if (type == OscillatorType::SQUARE)
        {
            static SignalSourceSquareWave ss;
            
            osc.SetSignalSource(&ss.GetSample);
        }
        else if (type == OscillatorType::TRIANGLE)
        {
            static SignalSourceTriangleWave ss;
            
            osc.SetSignalSource(&ss.GetSample);
        }
    }
    
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Main Synthesis Loop
    //
    ///////////////////////////////////////////////////////////////////////

    static void OnInterrupt()
    {
        // Debug
        PAL.DigitalToggle(dbg_);
        
        // Apply the LFO for next time
        if (lfoEnabled_)
        {
            int8_t lfoVal = lfo_.GetNextSample();
            
            // Calculate a percent increase in frequency for the oscillator
            // LFO value varies between -128 and 127.
            // Shift to 0-255.
            // Then consider that a percentage over 255 (a Q08 type).
            // This is the "intensity" factor.
            
            Q08 lfoIntensity = (uint8_t)(lfoVal + 128);
            
            
            // Objective is to use this intensity to determine how much to
            // increase the frequency of the given oscillators.
            // EG:
            // - LFO at 15
            //   - that's 143 / 255 = 56%
            // - OSC1 freq is 100Hz
            // - OSC1 freq should go to 100Hz + (100Hz * 56%) = 156Hz
            //
            // However, we want to first allow allow user ability to dampen that
            // intensity factor.
            //
            // That is, what percent of the current intensity should be
            // applied?
            //
            // Allow a user-tunable control over this.
            
            Q08 pctIncrease = lfoIntensity * lfoVibratoPct_;
            
            
            // Now apply to each oscillator
            osc1_.ApplyFrequencyOffsetPctIncreaseFromBase(pctIncrease);
            osc2_.ApplyFrequencyOffsetPctIncreaseFromBase(pctIncrease);
        }
        
        
        
        
        // Get current raw oscillator value
        int8_t osc1Val = osc1_.GetNextSample();
        int8_t osc2Val = osc2_.GetNextSample();

        // Prepare to store single value representing the oscillator set
        int8_t oscVal = 0;
        
        // Scale if both enabled
        if (osc1Enabled_ && osc2Enabled_)
        {
            // Scale and combine oscillator values
            osc1Val = osc1Val * osc1Factor_;
            osc2Val = osc2Val * osc2Factor_;
            
            oscVal = osc1Val + osc2Val;
        }
        else if (osc1Enabled_)
        {
            oscVal = osc1Val;
        }
        else if (osc2Enabled_)
        {
            oscVal = osc2Val;
        }
        










        
        /*
         * Come back for envelope stuff later
         *
        
        // Get envelope value and apply
        Q08 envVal = envADSR_.GetNextEnvelope();

        int8_t scaledVal = oscVal;
        if (envADSREnabled_)
        {
            PAL.DigitalToggle(dbg_);
            scaledVal = (oscVal * envVal);
        }
        */

        
        
        
        
        
        
        // Adjust to 0-255 range, but only use if some signal is in effect
        uint8_t val = 128 + oscVal;
        if (!osc1Enabled_ && !osc2Enabled_)
        {
            val = 0;
        }
        
        // Create analog signal
        PORTD = val;
    }
    

    // Debug
    static Pin dbg_;
    
    static uint8_t phaseLock_;
    
    static SignalOscillator osc1_;
    static uint8_t          osc1Enabled_;
    static Q08              osc1Factor_;
    
    static SignalOscillator osc2_;
    static uint8_t          osc2Enabled_;
    static Q08              osc2Factor_;
    
    static SignalOscillator lfo_;
    static uint8_t          lfoEnabled_;
    static Q08              lfoVibratoPct_;
    
    static SignalEnvelopeADSR envADSR_;
    static uint8_t            envADSREnabled_;
    
    static constexpr TimerChannel *tca_ = TimerClass::GetTimerChannelA();
    
    static TimedEventHandlerDelegate ted_;
};


template <typename TimerClass>
Pin SynthesizerVoice<TimerClass>::dbg_(14, LOW);

template <typename TimerClass>
uint8_t SynthesizerVoice<TimerClass>::phaseLock_ = 0;

template <typename TimerClass>
SignalOscillator SynthesizerVoice<TimerClass>::osc1_;
template <typename TimerClass>
uint8_t SynthesizerVoice<TimerClass>::osc1Enabled_ = 1;
template <typename TimerClass>
Q08 SynthesizerVoice<TimerClass>::osc1Factor_ = 0.5;

template <typename TimerClass>
SignalOscillator SynthesizerVoice<TimerClass>::osc2_;
template <typename TimerClass>
uint8_t SynthesizerVoice<TimerClass>::osc2Enabled_ = 1;
template <typename TimerClass>
Q08 SynthesizerVoice<TimerClass>::osc2Factor_ = 0.5;

template <typename TimerClass>
SignalOscillator SynthesizerVoice<TimerClass>::lfo_;
template <typename TimerClass>
uint8_t SynthesizerVoice<TimerClass>::lfoEnabled_ = 1;
template <typename TimerClass>
Q08 SynthesizerVoice<TimerClass>::lfoVibratoPct_ = 0.5;


template <typename TimerClass>
SignalEnvelopeADSR SynthesizerVoice<TimerClass>::envADSR_;
template <typename TimerClass>
uint8_t SynthesizerVoice<TimerClass>::envADSREnabled_ = 1;

template <typename TimerClass>
TimedEventHandlerDelegate SynthesizerVoice<TimerClass>::ted_;





#endif  // __SYNTHESIZER_VOICE_H__
















