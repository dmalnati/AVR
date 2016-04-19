#ifndef __KEYPAD_12_BUTTON_H__
#define __KEYPAD_12_BUTTON_H__


#include "PAL.h"
#include "Function.h"
#include "TimedEventHandler.h"


struct Keypad12ButtonConfig
{
    uint8_t pinK1;
    uint8_t pinK2;
    uint8_t pinK3;
    uint8_t pinK4;
    uint8_t pinK5;
    uint8_t pinK6;
    uint8_t pinK7;
    uint8_t pinK8;
    uint8_t pinK9;
    uint8_t pinK10;
};


class Keypad12Button
: private TimedEventHandler
{
public:
    static const uint8_t C_IDLE  = 0;
    static const uint8_t C_TIMED = 1;
    static const uint8_t C_INTER = 0;
    
private:
    static const uint32_t DEFAULT_POLL_INTERVAL_MS = 20;
    static const uint8_t  BUTTON_COUNT             = 12;
    
public:
    Keypad12Button(const Keypad12ButtonConfig cfg)
    : buttonStateList_{
        { cfg.pinK5, cfg.pinK1,  '1' },
        { cfg.pinK6, cfg.pinK1,  '2' },
        { cfg.pinK7, cfg.pinK1,  '3' },
        { cfg.pinK5, cfg.pinK2,  '4' },
        { cfg.pinK6, cfg.pinK2,  '5' },
        { cfg.pinK7, cfg.pinK2,  '6' },
        { cfg.pinK5, cfg.pinK4,  '7' },
        { cfg.pinK6, cfg.pinK4,  '8' },
        { cfg.pinK7, cfg.pinK3,  '9' },
        { cfg.pinK7, cfg.pinK4,  '0' },
        { cfg.pinK9, cfg.pinK8,  '*' },
        { cfg.pinK9, cfg.pinK10, '#' }
    }
    {
        // Set initial pin modes and states
        for (uint8_t i = 0; i < BUTTON_COUNT; ++i)
        {
            ButtonState &b = buttonStateList_[i];
            
            PAL.PinMode(b.pinInput, INPUT_PULLUP);
            PAL.PinMode(b.pinOutput, OUTPUT);
            PAL.DigitalWrite(b.pinOutput, HIGH);
        }
    }

    void Init(function<void(char)> cbFn,
              uint32_t             pollIntervalMs = DEFAULT_POLL_INTERVAL_MS)
    {
        RegisterForTimedEventInterval(pollIntervalMs);
        
        cbFn_ = cbFn;
    }

private:
    struct ButtonState
    {
        ButtonState(uint8_t pinInput, uint8_t pinOutput, char symbol)
        : pinInput(pinInput)
        , pinOutput(pinOutput)
        , symbol(symbol)
        , pressState(0)
        {
            // Nothing to do
        }
        
        Pin      pinInput;
        Pin      pinOutput;
        char     symbol;
        uint8_t  pressState;
    };

    virtual void OnTimedEvent()
    {
        HandleKeyPresses();
    }
    
    void HandleKeyPresses()
    {
        // Scan for pressed keys
        for (uint8_t i = 0; i < BUTTON_COUNT; ++i)
        {
            ButtonState &b = buttonStateList_[i];
            
            // Logic is "active low," so invert the read
            PAL.DigitalWrite(b.pinOutput, LOW);
            uint8_t pressState = !PAL.DigitalRead(b.pinInput);
            PAL.DigitalWrite(b.pinOutput, HIGH);
            
            // See if state changed and is active
            if (pressState != b.pressState)
            {
                if (pressState)
                {
                    // Button is pressed -- fire callback
                    cbFn_(b.symbol);
                }
                
                b.pressState = pressState;
            }
        }

    }
    
    function<void(char)> cbFn_;

    ButtonState buttonStateList_[BUTTON_COUNT];
};





#endif  // __KEYPAD_12_BUTTON_H__


