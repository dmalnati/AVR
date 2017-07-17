#ifndef __FUNCTION_GENERATOR_H__
#define __FUNCTION_GENERATOR_H__


#include "SignalSourceNoneWave.h"
#include "SignalSourceSineWave.h"
#include "SignalSourceSawtoothRightWave.h"
#include "SignalSourceSawtoothLeftWave.h"
#include "SignalSourceSquareWave.h"
#include "SignalSourceTriangleWave.h"
#include "SignalOscillator.h"


class FunctionGenerator
{
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

    FunctionGenerator()
    {
        // Set up oscillators
        SetOscillator1WaveType(OscillatorType::SINE);
        SetOscillator2WaveType(OscillatorType::NONE);
        
        // Set up LFO
        SetLFOWaveType(OscillatorType::SINE);
        
        // Debug
        PAL.PinMode(dbg_, OUTPUT);
    }
    
    ~FunctionGenerator()
    {
        // Nothing to do
    }
    
    void SetSampleRate(uint16_t sampleRate)
    {
        // Set up oscillator
        osc1_.SetSampleRate(sampleRate);
        osc2_.SetSampleRate(sampleRate);
        
        // Set up LFO
        lfo_.SetSampleRate(sampleRate);
        
        // Reset state
        Reset();
        
        // Debug
        PAL.PinMode(dbg_, OUTPUT);
    }
    
    void Reset()
    {
        // Reset state of oscillators
        SyncAllOscillators();
    }
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Mode
    //
    ///////////////////////////////////////////////////////////////////////
    
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
    
    void SetLFOTromoloPct(uint8_t tromoloPct)
    {
        lfoTromoloPct_ = tromoloPct;
    }
    
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Main Synthesis
    //
    ///////////////////////////////////////////////////////////////////////

    static uint8_t GetNextValue()
    {
        uint8_t retVal = 0;
        
        // Debug
        PAL.DigitalToggle(dbg_);
        
        
        // LFO signal processing.
        //
        // LFO value varies between -128 and 127.
        // Shift to 0-255.
        // Then consider that a percentage over 255 (a Q08 type).
        // This is the "intensity" factor.
        // Two values are maintained, in-phase and 180-degrees out-of-phase,
        // which supports both vibrato and tremolo.
        // We want vibrato to be most intense at the same time tromolo is at
        // peak amplitude, both phases are needed for that.
        int8_t lfoVal = lfo_.GetNextSample();
        
        uint8_t lfoValPhase0   = (lfoVal + 128);
        uint8_t lfoValPhase180 = 255 - lfoValPhase0;

        Q08 lfoIntensityPhase0   = lfoValPhase0;
        Q08 lfoIntensityPhase180 = lfoValPhase180;
        
        // LFO Vibrato
        if (lfoEnabled_)
        {
            // Calculate a percent increase in frequency for the oscillator.
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
            
            Q08 pctIncrease = lfoIntensityPhase0 * lfoVibratoPct_;
            
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
        
        retVal = oscVal;
        

        // LFO Tromolo
        if (lfoEnabled_)
        {
            retVal = oscVal - (oscVal * (lfoIntensityPhase180 * lfoTromoloPct_));
        }


        return retVal;
    }
    
    
private:

    ///////////////////////////////////////////////////////////////////////
    //
    // Oscillator Control
    //
    ///////////////////////////////////////////////////////////////////////
    
    void SyncAllOscillators()
    {
        osc1_.Reset();
        osc2_.Reset();
        lfo_.Reset();
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
    
    
private:

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
    static Q08              lfoTromoloPct_;
};


Pin              FunctionGenerator::dbg_(14, LOW);

uint8_t          FunctionGenerator::phaseLock_ = 0;

SignalOscillator FunctionGenerator::osc1_;
uint8_t          FunctionGenerator::osc1Enabled_ = 1;
Q08              FunctionGenerator::osc1Factor_  = 0.5;

SignalOscillator FunctionGenerator::osc2_;
uint8_t          FunctionGenerator::osc2Enabled_ = 1;
Q08              FunctionGenerator::osc2Factor_  = 0.5;

SignalOscillator FunctionGenerator::lfo_;
uint8_t          FunctionGenerator::lfoEnabled_    = 1;
Q08              FunctionGenerator::lfoVibratoPct_ = 0.5;
Q08              FunctionGenerator::lfoTromoloPct_ = 0.5;




#endif  // __FUNCTION_GENERATOR_H__
















