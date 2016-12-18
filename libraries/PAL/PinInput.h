#ifndef __PIN_INPUT_H__
#define __PIN_INPUT_H__


/*
 * Makes pin input available active HIGH or active LOW.
 * Eases translation between a high voltage and a logical 'on' or 'off' state.
 * Assumes caller wants to see transitions to/from both levels.
 *
 */


#include "PAL.h"
#include "Function.h"
#include "InterruptEventHandler.h"


class PinInput
: private InterruptEventHandler
{
public:
    PinInput(uint8_t pin,
             uint8_t mode        = LEVEL_RISING,
             uint8_t activeLevel = LOW)
    : pin_(pin)
    , activeLevel_(activeLevel)
    , mode_(mode)
    {
        // Enable to get the pin mode set correctly so GetLogicLevel works, then
        // Disable to prevent events from firing until application
        // ready.
        Enable();
        Disable();
    }
    
    void SetCallback(function<void(uint8_t logicLevel)> &&cbFn)
    {
        cbFn_ = cbFn;
    }
    
    uint8_t GetLogicLevel(uint8_t forceRefresh = 0)
    {
        uint8_t retVal = logicLevelActual_;
        
        if (forceRefresh)
        {
            logicLevelActual_ = PAL.DigitalRead(pin_);
            
            retVal = logicLevelActual_;
        }
        
        if (activeLevel_ == LOW)
        {
            retVal = !retVal;
        }
        
        return retVal;
    }
    
    void Disable()
    {
        DeRegisterForInterruptEvent();
    }
    
    void Enable()
    {
        // Assume an undefined transition mode means changing from
        // logical 0 to logical 1

        if (activeLevel_ == HIGH)
        {
            // High voltage == logical 1
            
            if (mode_ == LEVEL_UNDEFINED || mode_ == LEVEL_RISING)
            {
                RegisterForInterruptEvent(pin_, LEVEL_RISING);
            }
            else if (mode_ == LEVEL_FALLING)
            {
                PAL.PinMode(pin_, INPUT_PULLUP);
                RegisterForInterruptEvent(pin_, LEVEL_FALLING);
            }
            else
            {
                PAL.PinMode(pin_, INPUT_PULLUP);
                RegisterForInterruptEvent(pin_, LEVEL_RISING_AND_FALLING);
            }
        }
        else
        {
            // Low voltage == logical 1
            
            if (mode_ == LEVEL_RISING)
            {
                PAL.PinMode(pin_, INPUT_PULLUP);
                RegisterForInterruptEvent(pin_, LEVEL_FALLING);
            }
            else if (mode_ == LEVEL_UNDEFINED || mode_ == LEVEL_FALLING)
            {
                RegisterForInterruptEvent(pin_, LEVEL_RISING);
            }
            else
            {
                PAL.PinMode(pin_, INPUT_PULLUP);
                RegisterForInterruptEvent(pin_, LEVEL_RISING_AND_FALLING);
            }
        }
        
        logicLevelActual_ = PAL.DigitalRead(pin_);
    }
    
private:
    virtual void OnInterruptEvent(uint8_t logicLevel)
    {
        logicLevelActual_ = logicLevel;
        
        cbFn_(GetLogicLevel());
    }

    uint8_t                 pin_;
    uint8_t                 logicLevelActual_;
    uint8_t                 activeLevel_;
    uint8_t                 mode_;
    function<void(uint8_t)> cbFn_;
};


#endif  // __PIN_INPUT_H__