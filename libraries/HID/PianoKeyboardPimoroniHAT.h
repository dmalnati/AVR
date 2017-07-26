#ifndef __PIANO_KEYBOARD_PIMORONI_HAT_H__
#define __PIANO_KEYBOARD_PIMORONI_HAT_H__


#include "TWI.h"
#include "Function.h"
#include "TimedEventHandler.h"
#include "SensorCapacitiveTouchCAP1188.h"


class PianoKeyboardPimoroniHAT
{
    static const uint16_t DEFAULT_POLL_PERIOD = 20;
    
    static const uint8_t CAP1_ADDR = 0x28;
    static const uint8_t CAP2_ADDR = 0x2B;
    
public:

    PianoKeyboardPimoroniHAT()
    : cap1_(CAP1_ADDR)
    {
        // Nothing to do
    }

    void Init()
    {
        ted_.SetCallback([this](){
            OnPoll();
        });
        ted_.RegisterForTimedEventInterval(DEFAULT_POLL_PERIOD);
    }

    void SetCallbackOnKeyDown(function<void(uint8_t)> cbFnOnKeyDown)
    {
        cbFnOnKeyDown_ = cbFnOnKeyDown;
    }
    
    void SetCallbackOnKeyUp(function<void(uint8_t)> cbFnOnKeyUp)
    {
        cbFnOnKeyUp_ = cbFnOnKeyUp;
    }
    
    void SetCallbackOnInstrumentChangeKeyPress(function<void()> cbFnOnInstrumentChangeKeyPress)
    {
        cbFnOnInstrumentChangeKeyPress_ = cbFnOnInstrumentChangeKeyPress;
    }
    
    void SetCallbackOnOctaveKeyUpPress(function<void()> cbFnOnOctaveKeyUpPress)
    {
        cbFnOnOctaveKeyUpPress_ = cbFnOnOctaveKeyUpPress;
    }
    
    void SetCallbackOnOctaveKeyDownPress(function<void()> cbFnOnOctaveKeyDownPress)
    {
        cbFnOnOctaveKeyDownPress_ = cbFnOnOctaveKeyDownPress;
    }


private:

    void OnPoll()
    {
        
    }
    
    SensorCapacitiveTouchCAP1188 cap1_;
    
    function<void(uint8_t)>  cbFnOnKeyDown_;
    function<void(uint8_t)>  cbFnOnKeyUp_;
    function<void()>         cbFnOnInstrumentChangeKeyPress_;
    function<void()>         cbFnOnOctaveKeyUpPress_;
    function<void()>         cbFnOnOctaveKeyDownPress_;

    TimedEventHandlerDelegate ted_;
};



#endif  // __PIANO_KEYBOARD_PIMORONI_HAT_H__