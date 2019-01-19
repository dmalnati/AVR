#ifndef __ROTARY_ENCODER_WITH_BUTTON_INPUT_H__
#define __ROTARY_ENCODER_WITH_BUTTON_INPUT_H__


#include "Function.h"
#include "PinInput.h"
#include "RotaryEncoderInput.h"


class RotaryEncoderWithButtonInput
{
public:
    RotaryEncoderWithButtonInput(uint8_t pinA, uint8_t pinB, uint8_t pinButton)
    : rotaryEncoder_(pinA, pinB)
    , pinInput_(pinButton)
    {
        // Nothing to do
    }
    
    void Enable()
    {
        rotaryEncoder_.Enable();
        pinInput_.Enable();
    }
    
    void Disable()
    {
        rotaryEncoder_.Disable();
        pinInput_.Disable();
    }
    
    void SetCallbackOnRotate(function<void(int8_t leftOrRight)> cbFn)
    {
        rotaryEncoder_.SetCallback(cbFn);
    }
    
    void SetCallbackOnPressChange(function<void(uint8_t logicLevel)> &&cbFn)
    {
        pinInput_.SetCallback(cbFn);
    }


private:

    RotaryEncoderInput rotaryEncoder_;
    PinInput           pinInput_;
};


#endif  // __ROTARY_ENCODER_WITH_BUTTON_INPUT_H__









