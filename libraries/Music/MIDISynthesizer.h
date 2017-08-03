#ifndef __MIDI_SYNTHESIZER_H__
#define __MIDI_SYNTHESIZER_H__


#include "Function.h"

#include "Synthesizer.h"


class MIDISynthesizer
: public Synthesizer
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
        else if (cmd.type == MIDICommand::Type::PROGRAM_CHANGE)
        {
            // Ignore data, just cycle presets
            Synthesizer::CycleToNextInstrument();
        }
    }
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Configuration
    //
    ///////////////////////////////////////////////////////////////////////
    
    void SetCfgItem(CfgItem c)
    {
        Synthesizer::SetCfgItem(c);
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
        Synthesizer::Note  note       = Synthesizer::Note::C;
        
        if      (noteNumber ==  0) { note = Synthesizer::Note::C;       }
        else if (noteNumber ==  1) { note = Synthesizer::Note::C_SHARP; }
        else if (noteNumber ==  2) { note = Synthesizer::Note::D;       }
        else if (noteNumber ==  3) { note = Synthesizer::Note::D_SHARP; }
        else if (noteNumber ==  4) { note = Synthesizer::Note::E;       }
        else if (noteNumber ==  5) { note = Synthesizer::Note::F;       }
        else if (noteNumber ==  6) { note = Synthesizer::Note::F_SHARP; }
        else if (noteNumber ==  7) { note = Synthesizer::Note::G;       }
        else if (noteNumber ==  8) { note = Synthesizer::Note::G_SHARP; }
        else if (noteNumber ==  9) { note = Synthesizer::Note::A;       }
        else if (noteNumber == 10) { note = Synthesizer::Note::A_SHARP; }
        else if (noteNumber == 11) { note = Synthesizer::Note::B;       }
        
        NoteAndOctave retVal = { note, octave };
        
        return retVal;
    }
};


#endif  // __MIDI_SYNTHESIZER_H__








