#ifndef __MIDI_SYNTHESIZER_H__
#define __MIDI_SYNTHESIZER_H__


#include "Synthesizer.h"


class MIDISynthesizer
: private Synthesizer
{
public:

    void Init(uint16_t sampleRate)
    {
        Synthesizer::SetSampleRate(sampleRate);
    }
    
    void Start()
    {
        Synthesizer::Start();
    }
    
    void Stop()
    {
        Synthesizer::Stop();
    }
    
    void ProcessCommand(const MidiCommand &/*cmd*/)
    {
        
    }


    
    // users of this class will:
    // - have an input stream of music data
    //   - probably midi synthesizers (an owner of this class?)
    //   - serial debugger too
    // 
    
    
    

private:


};




#endif  // __MIDI_SYNTHESIZER_H__








