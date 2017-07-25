#ifndef __MIDI_COMMAND_FROM_PIANO_KEYBOARD_PIMORONI_HAT_H__
#define __MIDI_COMMAND_FROM_PIANO_KEYBOARD_PIMORONI_HAT_H__


#include "PianoKeyboardPimoroniHAT.h"
#include "MIDICommandToSerial.h"


class MIDICommandFromPianoKeyboardPimoroniHAT
{
public:

    MIDICommandFromPianoKeyboardPimoroniHAT(PianoKeyboardPimoroniHAT &hat)
    : hat_(hat)
    {
        // Nothing to do
    }

    void Init()
    {
        hat_.SetCallbackOnKeyDown([this](uint8_t){
            
        });
        
        hat_.SetCallbackOnKeyUp([this](uint8_t){
            
        });
    }

private:
    PianoKeyboardPimoroniHAT  &hat_;
    MIDICommandToSerial        mcToSerial_;
};


#endif  // __MIDI_COMMAND_FROM_PIANO_KEYBOARD_PIMORONI_HAT_H__





