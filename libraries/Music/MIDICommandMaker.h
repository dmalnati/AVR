#ifndef __MIDI_COMMAND_MAKER_H__
#define __MIDI_COMMAND_MAKER_H__


#include "MIDICommand.h"


class MIDICommandMaker
{
    static const uint8_t DEFAULT_CHANNEL  = 0;
    static const uint8_t DEFAULT_VELOCITY = 127;
    
public:

    MIDICommand MakeNoteOn(uint8_t noteNumber, uint8_t octave)
    {
        MIDICommand mc;
        
        mc.type    = MIDICommand::Type::NOTE_ON;
        mc.channel = DEFAULT_CHANNEL;
        mc.param1  = (octave * 12) + noteNumber;
        mc.param2  = DEFAULT_VELOCITY;
        
        return mc;
    }
    
    MIDICommand MakeNoteOff(uint8_t noteNumber, uint8_t octave)
    {
        MIDICommand mc;
        
        mc.type    = MIDICommand::Type::NOTE_OFF;
        mc.channel = DEFAULT_CHANNEL;
        mc.param1  = (octave * 12) + noteNumber;
        mc.param2  = DEFAULT_VELOCITY;
        
        return mc;
    }
    
    MIDICommand MakeProgramChange(uint8_t program)
    {
        MIDICommand mc;
        
        mc.type    = MIDICommand::Type::PROGRAM_CHANGE;
        mc.channel = DEFAULT_CHANNEL;
        mc.param1  = program;
        
        return mc;
    }

};



#endif  // __MIDI_COMMAND_MAKER_H__