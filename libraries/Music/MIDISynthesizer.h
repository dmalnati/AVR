#ifndef __MIDI_SYNTHESIZER_H__
#define __MIDI_SYNTHESIZER_H__


#include "Function.h"

#include "Synthesizer.h"


class MIDISynthesizer
: private Synthesizer
{
public:

    ///////////////////////////////////////////////////////////////////////
    //
    // Initialization
    //
    ///////////////////////////////////////////////////////////////////////

    void Init()
    {
        Synthesizer::Init();
    }
    
    void Start()
    {
        Synthesizer::Start();
    }
    
    void Stop()
    {
        Synthesizer::Stop();
    }
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Main Interface
    //
    ///////////////////////////////////////////////////////////////////////
    
    void ProcessCommand(MIDICommand cmd)
    {
        // Also consider NOTE_ON w/ velocity == 0 a NOTE_OFF
        if (cmd.type == MIDICommand::Type::NOTE_OFF ||
           (cmd.type == MIDICommand::Type::NOTE_ON && cmd.param2 == 0))
        {
            Synthesizer::OnKeyUp();
        }
        else if (cmd.type == MIDICommand::Type::NOTE_ON)
        {
            NoteAndOctave no = GetNoteAndOctave(cmd.param1);
            
            Synthesizer::OnKeyDown(no.note, no.octave);
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
        default:
            Synthesizer::SetCfgItem(c);
        }
    }

    
private:

    struct NoteAndOctave
    {
        Synthesizer::Note  note;
        int8_t             octave;
    };
    
    NoteAndOctave GetNoteAndOctave(uint8_t param1)
    {
        int8_t             octave     = param1 / 12;
        uint8_t            noteNumber = param1 - (octave * 12);
        Synthesizer::Note  note       = NOTE_NUMBER__NOTE[noteNumber];
        
        NoteAndOctave retVal = { note, octave };
        
        return retVal;
    }
    
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Members
    //
    ///////////////////////////////////////////////////////////////////////

    static const uint8_t NOTE_NUMBER_COUNT = 19;
    static const Synthesizer::Note 
        NOTE_NUMBER__NOTE[MIDISynthesizer::NOTE_NUMBER_COUNT];


};

const Synthesizer::Note
MIDISynthesizer::NOTE_NUMBER__NOTE[MIDISynthesizer::NOTE_NUMBER_COUNT] =
{
    Synthesizer::Note::C,
    Synthesizer::Note::C_SHARP,
    Synthesizer::Note::D,
    Synthesizer::Note::D_SHARP,
    Synthesizer::Note::E,
    Synthesizer::Note::F,
    Synthesizer::Note::F_SHARP,
    Synthesizer::Note::G,
    Synthesizer::Note::G_SHARP,
    Synthesizer::Note::A,
    Synthesizer::Note::A_SHARP,
    Synthesizer::Note::B,
};


#endif  // __MIDI_SYNTHESIZER_H__








