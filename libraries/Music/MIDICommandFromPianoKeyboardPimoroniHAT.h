#ifndef __MIDI_COMMAND_FROM_PIANO_KEYBOARD_PIMORONI_HAT_H__
#define __MIDI_COMMAND_FROM_PIANO_KEYBOARD_PIMORONI_HAT_H__


#include "PianoKeyboardPimoroniHAT.h"
#include "MIDICommandMaker.h"
#include "MIDICommandTo@fix@Serial.h"


class MIDICommandFromPianoKeyboardPimoroniHAT
{
    static const uint8_t OCTAVE_MIN     = 1;
    static const uint8_t OCTAVE_MAX     = 8;
    static const uint8_t OCTAVE_DEFAULT = 4;

    
public:

    MIDICommandFromPianoKeyboardPimoroniHAT()
    : octave_(OCTAVE_DEFAULT)
    {
        // Nothing to do
    }

    void Init()
    {
        hat_.SetCallbackOnKeyDown([this](uint8_t noteNumber){
            MakeAndSendMIDICommandNoteOn(noteNumber, octave_);
        });
        
        hat_.SetCallbackOnKeyUp([this](uint8_t noteNumber){
            MakeAndSendMIDICommandNoteOff(noteNumber, octave_);
        });
        
        hat_.SetCallbackOnInstrumentChangeKeyPress([this](){
            MakeAndSendMIDICommandProgramChange();
        });
        
        hat_.SetCallbackOnOctaveKeyUpPress([this](){
            octave_ = GetConstrainedOctave(octave_ + 1);
        });
        
        hat_.SetCallbackOnOctaveKeyDownPress([this](){
            octave_ = GetConstrainedOctave(octave_ - 1);
        });
        
        hat_.Init();
    }
    
    PianoKeyboardPimoroniHAT &GetHat()
    {
        return hat_;
    }

private:

    void MakeAndSendMIDICommandNoteOn(uint8_t noteNumber, uint8_t octave)
    {
        mcToSerial_.SendCommand(mcm_.MakeNoteOn(noteNumber, octave));
    }
    
    void MakeAndSendMIDICommandNoteOff(uint8_t noteNumber, uint8_t octave)
    {
        mcToSerial_.SendCommand(mcm_.MakeNoteOff(noteNumber, octave));
    }
    
    void MakeAndSendMIDICommandProgramChange()
    {
        mcToSerial_.SendCommand(mcm_.MakeProgramChange(0));
    }
    
    uint8_t GetConstrainedOctave(uint8_t octaveInput)
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
        
        return octave;
    }

    PianoKeyboardPimoroniHAT  hat_;
    MIDICommandMaker          mcm_;
    MIDICommandToSerial       mcToSerial_;
    uint8_t                   octave_;
};


#endif  // __MIDI_COMMAND_FROM_PIANO_KEYBOARD_PIMORONI_HAT_H__





