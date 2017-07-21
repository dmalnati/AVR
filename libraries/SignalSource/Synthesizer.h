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
    
public:

    enum class Note : uint8_t
    {
        C       =  0,
        C_SHARP =  1,
        D_FLAT  =  2,
        D       =  3,
        D_SHARP =  4,
        E_FLAT  =  5,
        E       =  6,
        F_FLAT  =  7,
        E_SHARP =  8,
        F       =  9,
        F_SHARP = 10,
        G_FLAT  = 11,
        G       = 12,
        G_SHARP = 13,
        A_FLAT  = 14,
        A       = 15,
        A_SHARP = 16,
        B_FLAT  = 17,
        B       = 18
    };

    
public:

    ///////////////////////////////////////////////////////////////////////
    //
    // Initialization
    //
    ///////////////////////////////////////////////////////////////////////
    
    void Init()
    {
        // Determine sample rate and configure SV
        // calculate duration per sample, over many samples, determine
        // appropriate sample rate to achieve, for example:
        // - 90% utilization, 10% headroom
        const uint16_t SAMPLE_RATE = 10000;
        SynthesizerVoiceClass::SetSampleRate(SAMPLE_RATE);
        
        SetDefaultValues();
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
        // Convert note into a frequency
        uint16_t freq = ScaleFreqToOctave(NOTE__FREQ[(uint8_t)n]);
        
        SetCfgItem({SET_OSCILLATOR_1_FREQUENCY, freq});
        SetCfgItem({SET_OSCILLATOR_2_FREQUENCY, freq + 3});
        
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
        switch (c.type)
        {
        case SET_OCTAVE:
            SetOctave((uint8_t)c);
            break;
            
        default:
            SynthesizerVoiceClass::SetCfgItem(c);
        }
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
        uint16_t retVal = freq * (double)octave_ / (double)OCTAVE_DEFAULT;
        
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

    
    static const uint8_t NOTE_COUNT = 19;
    static const double  NOTE__FREQ[Synthesizer::NOTE_COUNT];
};


// https://www.key-notes.com/blog/layout-of-piano-keys
// https://www.key-notes.com/blog/piano-notes-chart
// https://www.key-notes.com/blog/piano-key-chart
// https://en.wikipedia.org/wiki/C_(musical_note)
// https://en.wikipedia.org/wiki/Piano_key_frequencies
// C D E F G A B C

// Note that these are in the OCTAVE_DEFAULT == 4
const double Synthesizer::NOTE__FREQ[Synthesizer::NOTE_COUNT] =
{
    261.626, // C      
    277.183, // C_SHARP
    277.183, // D_FLAT 
    293.665, // D      
    311.127, // D_SHARP
    311.127, // E_FLAT 
    329.628, // E      
    329.628, // F_FLAT 
    349.228, // E_SHARP
    349.228, // F      
    369.994, // F_SHARP
    369.994, // G_FLAT 
    391.995, // G      
    415.305, // G_SHARP
    415.305, // A_FLAT 
    440.000, // A      
    466.164, // A_SHARP
    466.164, // B_FLAT 
    493.883, // B      
};





#endif  // __SYNTHESIZER_H__








