#ifndef __SYNTHESIZER_H__
#define __SYNTHESIZER_H__


#include "Timer2.h"

#include "SynthesizerVoice.h"


enum
{
    SET_OCTAVE = 51,
};


class Synthesizer
: private SynthesizerVoice<Timer2>
{
    using SynthesizerVoiceClass = SynthesizerVoice<Timer2>;
    using FnPtr                 = void (Synthesizer::*)();
    
    static const uint8_t OCTAVE_MIN     = 1;
    static const uint8_t OCTAVE_MAX     = 8;
    static const uint8_t OCTAVE_DEFAULT = 4;
    
    static const uint8_t DEFAULT_USE_PCT_CPU = 90;
    
public:

    enum class Note : uint8_t
    {
        C       =  0,
        C_SHARP =  1,
        D       =  2,
        D_SHARP =  3,
        E       =  4,
        F       =  5,
        F_SHARP =  6,
        G       =  7,
        G_SHARP =  8,
        A       =  9,
        A_SHARP = 10,
        B       = 11
    };

    
public:

    Synthesizer()
    {
        SetDefaultValues();
    }

    ///////////////////////////////////////////////////////////////////////
    //
    // Initialization
    //
    ///////////////////////////////////////////////////////////////////////
    
    void Init(uint8_t usePctCpu = DEFAULT_USE_PCT_CPU)
    {
        // Normalize CPU utilization value
        if (usePctCpu > 100)
        {
            usePctCpu = 100;
        }
        else if (usePctCpu == 0)
        {
            usePctCpu = 1;
        }
        
        double pctCpuFloat = (double)usePctCpu / 100.0;

        // Measurements indicate on 8MHz and using Timer2 ISR, there is approx
        // 10us latency between ISR calls.
        double usPerIsr = ((double)PAL.GetCpuFreq() / 8000000.0) * 10;
        
        double usPerLoop             = SynthesizerVoiceClass::GetLoopDurationUs();
        double usPerLoopAdjusted     = usPerLoop + usPerIsr;
        double loopsPerSec           = 1000000 / usPerLoopAdjusted;
        double loopsPerSecWithMargin = loopsPerSec * pctCpuFloat;
        
        /*
        double usPerLoopWithMargin   = usPerLoop / pctCpuFloat;
        Serial.print("usPerIsr: ");              Serial.println(usPerIsr);
        Serial.print("usPerLoop: ");             Serial.println(usPerLoop);
        Serial.print("usPerLoopAdjusted: ");     Serial.println(usPerLoopAdjusted);
        Serial.print("loopsPerSec: ");           Serial.println(loopsPerSec);
        Serial.print("usePctCpu: ");             Serial.println(usePctCpu);
        Serial.print("loopsPerSecWithMargin: "); Serial.println(loopsPerSecWithMargin);
        Serial.print("usPerLoopWithMargin: ");   Serial.println(usPerLoopWithMargin);
        */
        
        SynthesizerVoiceClass::SetSampleRate(loopsPerSecWithMargin);
    }
    
    void Start()
    {
        SynthesizerVoiceClass::Start();
    }
    
    void Stop()
    {
        SynthesizerVoiceClass::Stop();
    }
    
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Main Interface
    //
    ///////////////////////////////////////////////////////////////////////
    
    void OnKeyDown()
    {
        SynthesizerVoiceClass::EnvelopeBeginAttack();
    }
    
    void OnKeyDown(Note n)
    {
        OnKeyDown(n, octave_);
    }
    
    void OnKeyDown(Note n, uint8_t octave)
    {
        // https://www.key-notes.com/blog/layout-of-piano-keys
        // https://www.key-notes.com/blog/piano-notes-chart
        // https://www.key-notes.com/blog/piano-key-chart
        // https://en.wikipedia.org/wiki/C_(musical_note)
        // https://en.wikipedia.org/wiki/Piano_key_frequencies
        // C D E F G A B C
        
        double freq = 1;
        
        uint8_t noteNumber = (uint8_t)n;
        if      (noteNumber ==  0) { freq = 261.626; } // C      
        else if (noteNumber ==  1) { freq = 277.183; } // C_SHARP
        else if (noteNumber ==  2) { freq = 293.665; } // D      
        else if (noteNumber ==  3) { freq = 311.127; } // D_SHARP
        else if (noteNumber ==  4) { freq = 329.628; } // E      
        else if (noteNumber ==  5) { freq = 349.228; } // F      
        else if (noteNumber ==  6) { freq = 369.994; } // F_SHARP
        else if (noteNumber ==  7) { freq = 391.995; } // G      
        else if (noteNumber ==  8) { freq = 415.305; } // G_SHARP
        else if (noteNumber ==  9) { freq = 440.000; } // A      
        else if (noteNumber == 10) { freq = 466.164; } // A_SHARP
        else if (noteNumber == 11) { freq = 493.883; } // B      
        
        SetCfgItem({SET_OCTAVE, octave});
        
        uint16_t freqInt = ScaleFreqToOctave(freq);
        
        SetCfgItem({SET_OSCILLATOR_1_FREQUENCY, freqInt});
        SetCfgItem({SET_OSCILLATOR_2_FREQUENCY, freqInt + 3});
        
        OnKeyDown();
    }
    
    void OnKeyUp()
    {
        SynthesizerVoiceClass::EnvelopeBeginRelease();
    }
    
    void CycleToNextInstrument()
    {
        ++instrumentPresetListIdx_;
        
        if (instrumentPresetListIdx_ == INSTRUMNET_PRESET_COUNT)
        {
            instrumentPresetListIdx_ = 0;
        }
        
        ApplyInstrumentConfiguration();
    }

    
    ///////////////////////////////////////////////////////////////////////
    //
    // Configuration
    //
    ///////////////////////////////////////////////////////////////////////
    
    void SetCfgItem(CfgItem c)
    {
        if (c.type == SET_OCTAVE)
        {
            SetOctave((uint8_t)c);
        }
        else
        {
            SynthesizerVoiceClass::SetCfgItem(c);
        }
    }
    
    uint8_t GetCfgItem(uint8_t type, CfgItem &c)
    {
        return SynthesizerVoiceClass::GetCfgItem(type, c);
    }


private:

    void SetDefaultValues()
    {
        SetCfgItem({SET_OCTAVE, (uint8_t)OCTAVE_DEFAULT});
    }
    
    void SetOctave(uint8_t octaveInput)
    {
        uint8_t octave = octaveInput;
        
        if (octave < OCTAVE_MIN)
        {
            octave = OCTAVE_MIN;
        }
        else if (octave > OCTAVE_MAX)
        {
            octave = OCTAVE_MAX;
        }
        
        octave_ = octave;
    }
    
    uint16_t ScaleFreqToOctave(const double freq) const
    {
        uint16_t retVal;
        
        if (octave_ == OCTAVE_DEFAULT)
        {
            retVal = freq;
        }
        else if (octave_ < OCTAVE_DEFAULT)
        {
            uint8_t diffOctave = OCTAVE_DEFAULT - octave_;
            
            retVal = freq / (1 << diffOctave);
        }
        else
        {
            uint8_t diffOctave = octave_ - OCTAVE_DEFAULT;
            
            retVal = freq * (1 << diffOctave);
        }
        
        return retVal;
    }
    
    
    // Keep presets outside the main logic
    #include "SynthesizerPresets.h"
    
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Members
    //
    ///////////////////////////////////////////////////////////////////////
    
    uint8_t octave_;
};




#endif  // __SYNTHESIZER_H__








