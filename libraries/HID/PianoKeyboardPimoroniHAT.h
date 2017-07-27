#ifndef __PIANO_KEYBOARD_PIMORONI_HAT_H__
#define __PIANO_KEYBOARD_PIMORONI_HAT_H__


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
    , cap2_(CAP2_ADDR)
    , touchedBitmapLast_(0)
    , isTouched_(0)
    , touchedKeyNumber_(0)
    , ignoreKeyCNextOctave_(1)
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
    
    void EnableLEDs()
    {
        cap1_.EnableLEDs();
        cap2_.EnableLEDs();
    }
    
    void DisableLEDs()
    {
        cap1_.DisableLEDs();
        cap2_.DisableLEDs();
    }
    
    void SetIgnoreKeyCNextOctave(uint8_t ignoreKeyCNextOctave)
    {
        ignoreKeyCNextOctave_ = ignoreKeyCNextOctave;
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
        // Get current state of touched keys
        uint8_t cap1Touched = cap1_.GetTouched();
        uint8_t cap2Touched = cap2_.GetTouched();
        
        uint16_t touchedBitmap = (cap2Touched << 8) | cap1Touched;
        
        // Examine bitmap for key changes
        uint8_t checkForTouchedKey = 0;
        
        if (isTouched_)
        {
            // Was being touched as of last time
            
            // Check if prior key still being touched, if yes, do nothing
            if (touchedBitmap & (1 << touchedKeyNumber_))
            {
                // Do nothing
            }
            else
            {
                // Prior key is now up, fire callback if not a special key
                if (touchedKeyNumber_ <= 12)
                {
                    cbFnOnKeyUp_(touchedKeyNumber_);
                }
                
                // Reset state to show no key is pressed
                isTouched_        = 0;
                touchedKeyNumber_ = 0;
                
                // Search for currently touched key
                checkForTouchedKey = 1;
            }
        }
        else
        {
            // Not touched as of last time
            
            // Search for currently touched key
            checkForTouchedKey = 1;
        }
        
        if (checkForTouchedKey)
        {
            // Find the first touched key, if present, and fire callback
            uint8_t cont = 1;
            for (int8_t i = 15; i >= 0 && cont; --i)
            {
                uint8_t isTouched = !!(touchedBitmap & (1 << i));
                
                if (isTouched)
                {
                    uint8_t trackTouch = 1;
                    
                    // First key found, fire callback, depending on type
                    if (i <= 12)
                    { 
                        if (ignoreKeyCNextOctave_ && i == 12)
                        {
                            // Do nothing
                            trackTouch = 0;
                        }
                        else if (ignoreKeyCNextOctave_ && i != 12)
                        {
                            cbFnOnKeyDown_(i);
                        }
                    }
                    else if (i == 13)
                    {
                        cbFnOnOctaveKeyDownPress_();
                    }
                    else if (i == 14)
                    {
                        cbFnOnOctaveKeyUpPress_();
                    }
                    else if (i == 15)
                    {
                        cbFnOnInstrumentChangeKeyPress_();
                    }
                    
                    if (trackTouch)
                    {
                        // Save state for next time
                        isTouched_        = 1;
                        touchedKeyNumber_ = i;
                    }
                    
                    // Break out of search
                    cont = 0;
                }
            }
        }
        
        // Save state for next time
        touchedBitmapLast_ = touchedBitmap;
    }
    
    /*
     * cap1 76543210
     * - 7 = C
     * - 6 = C_SHARP
     * - 5 = D
     * - 4 = D_SHARP
     * - 3 = E
     * - 2 = F
     * - 1 = F_SHARP
     * - 0 = G
     *
     * cap2 76543210
     * - 7 = G_SHARP
     * - 6 = A
     * - 5 = A_SHARP
     * - 4 = B
     * - 3 = C (next octave)
     * - 2 = octave down
     * - 1 = octave up
     * - 0 = next instrument
     *
     * But the key number scheme is treated like a zero-offset array of 16 vals
     *      0000000000111111
     *      0123456789012345
     * cap1 76543210
     * cap2         76543210
     *
     */
    
    SensorCapacitiveTouchCAP1188 cap1_;
    SensorCapacitiveTouchCAP1188 cap2_;
    
    uint16_t touchedBitmapLast_;
    
    uint8_t isTouched_;
    uint8_t touchedKeyNumber_;
    
    uint8_t ignoreKeyCNextOctave_;
    
    function<void(uint8_t)>  cbFnOnKeyDown_;
    function<void(uint8_t)>  cbFnOnKeyUp_;
    function<void()>         cbFnOnInstrumentChangeKeyPress_;
    function<void()>         cbFnOnOctaveKeyUpPress_;
    function<void()>         cbFnOnOctaveKeyDownPress_;

    TimedEventHandlerDelegate ted_;
};



#endif  // __PIANO_KEYBOARD_PIMORONI_HAT_H__