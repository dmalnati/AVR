#ifndef __MIDI_SYNTHESIZER_H__
#define __MIDI_SYNTHESIZER_H__


#include "Timer2.h"
#include "SynthesizerVoice.h"


class MIDISynthesizer
: private SynthesizerVoice<Timer2>
{
    using SynthesizerVoiceClass = SynthesizerVoice<Timer2>;
    
public:

    void Init(uint16_t sampleRate)
    {
        SynthesizerVoiceClass::SetSampleRate(sampleRate);
    }
    
    void Start()
    {
        dac_.Start();
    }
    
    void Stop()
    {
        dac_.Stop();
    }
    
    void ProcessCommand(const MidiCommand &/*cmd*/)
    {
        
    }


    // presets
    // octaves
    // music notes
    // 
    
    
    // users of this class will:
    // - have an input stream of music data
    //   - probably midi synthesizers (an owner of this class?)
    //   - serial debugger too
    // 
    
    
    

private:


};




#endif  // __MIDI_SYNTHESIZER_H__








