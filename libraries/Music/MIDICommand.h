#ifndef __MIDI_COMMAND_H__
#define __MIDI_COMMAND_H__


struct MIDICommand
{
    enum class Type : uint8_t
    {
        INVALID               = 0b11111111,
        NOTE_OFF              = 0b00000000,
        NOTE_ON               = 0b00000001,
        AFTERTOUCH            = 0b00000010,
        CONTROL_CHANGE        = 0b00000011,
        PROGRAM_CHANGE        = 0b00000100,
        CHANNEL_PRESSURE      = 0b00000101,
        PITCH_BEND            = 0b00000110,
        NON_MUSICAL_COMMANDS  = 0b00000111
    };
    
    Type    type    = Type::INVALID;
    uint8_t channel = 0;
    uint8_t param1  = 0;
    uint8_t param2  = 0;
};


#endif  // __MIDI_COMMAND_H__