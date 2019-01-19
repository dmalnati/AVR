#ifndef __ROTARY_ENCODER_H__
#define __ROTARY_ENCODER_H__


#include "PinInput.h"


class RotaryEncoderInput
{
public:

    RotaryEncoderInput(uint8_t pinA, uint8_t pinB)
    : pinInputA_(pinA, LEVEL_RISING_AND_FALLING)
    , pinInputB_(pinB, LEVEL_RISING_AND_FALLING)
    , firstPinUp_(0)
    , aWasHigh_(0)
    , bWasHigh_(0)
    {
        pinInputA_.SetCallback([this](uint8_t logicLevel){
            OnA(logicLevel);
        });
        pinInputB_.SetCallback([this](uint8_t logicLevel){
            OnB(logicLevel);
        });
    }
    
    void Enable()
    {
        pinInputA_.Enable();
        pinInputB_.Enable();
    }
    
    void Disable()
    {
        pinInputA_.Disable();
        pinInputB_.Disable();
    }
    
    // -1 refers to turning right
    //  1 refers to turning left
    void SetCallback(function<void(int8_t leftOrRight)> cbFn)
    {
        cbFn_ = cbFn;
    }
    
private:

    // Both have to go low after either went high before an event.
    // if A went high before B, turning right
    // if B went high before A, turning left

    void OnA(uint8_t logicLevel)
    {
        uint8_t logicLevelA = logicLevel;
        uint8_t logicLevelB = pinInputB_.GetLogicLevel();
        
        if (logicLevelA)
        {
            aWasHigh_ = 1;
            
            if (!logicLevelB)
            {
                firstPinUp_ = -1;
            }
        }
        else
        {
            if (!logicLevelB)
            {
                DoCallback();
            }
        }
    }
    
    void OnB(uint8_t logicLevel)
    {
        uint8_t logicLevelA = pinInputA_.GetLogicLevel();
        uint8_t logicLevelB = logicLevel;
        
        if (logicLevelB)
        {
            bWasHigh_ = 1;
            
            if (!logicLevelA)
            {
                firstPinUp_ = 1;
            }
        }
        else
        {
            if (!logicLevelA)
            {
                DoCallback();
            }
        }
    }
    
    void DoCallback()
    {
        if (firstPinUp_ != 0 && aWasHigh_ && bWasHigh_)
        {
            cbFn_(firstPinUp_);
        }
        
        firstPinUp_ = 0;
        aWasHigh_   = 0;
        bWasHigh_   = 0;
    }
    

private:

    PinInput pinInputA_;
    PinInput pinInputB_;

    int8_t  firstPinUp_;
    uint8_t aWasHigh_;
    uint8_t bWasHigh_;
    
    function<void(int8_t)> cbFn_;
};


#endif  // __ROTARY_ENCODER_H__










