#ifndef __MIDI_COMMAND_TO_SERIAL_H__
#define __MIDI_COMMAND_TO_SERIAL_H__


#include "MIDICommand.h"


class MIDICommandToSerial
{
public:

    void SendMsg(MIDICommand cmd)
    {
        uint8_t buf[3] = { 0 };
        
        if ((cmd.type == MIDICommand::Type::NOTE_ON) ||
             cmd.type == MIDICommand::Type::NOTE_OFF)
        {
            buf[0] = (uint8_t)((uint8_t)cmd.type << 4 | cmd.channel);
            buf[1] = cmd.param1;
            buf[2] = cmd.param2;
            
            Serial.write(buf, 3);
        }
    }

};


#endif  // __MIDI_COMMAND_TO_SERIAL_H__



