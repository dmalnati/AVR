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
    : InterruptEventHandler(pin, GetInterruptEventHandlerMode(activeLevel, mode))
    , pin_(pin)
    , activeLevel_(activeLevel)
    , mode_(mode)
    {
        // Enable to get the pin mode set correctly so GetLogicLevel works, then
        // Disable to prevent events from firing until application
        // ready.
        Enable();
        Disable();
    }
    
    void SetCallback(function<void(uint8_t logicLevel)> cbFn)
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
                // Nothing to do
            }
            else if (mode_ == LEVEL_FALLING)
            {
                PAL.PinMode(pin_, INPUT_PULLUP);
            }
            else
            {
                PAL.PinMode(pin_, INPUT_PULLUP);
            }
        }
        else
        {
            // Low voltage == logical 1
            
            if (mode_ == LEVEL_RISING)
            {
                PAL.PinMode(pin_, INPUT_PULLUP);
            }
            else if (mode_ == LEVEL_UNDEFINED || mode_ == LEVEL_FALLING)
            {
                // Nothing to do
            }
            else
            {
                PAL.PinMode(pin_, INPUT_PULLUP);
            }
        }
        
        RegisterForInterruptEvent();
        
        logicLevelActual_ = PAL.DigitalRead(pin_);
    }
    
private:

    uint8_t GetInterruptEventHandlerMode(uint8_t activeLevel, uint8_t mode)
    {
        uint8_t retVal = LEVEL_FALLING;
        
        // Assume an undefined transition mode means changing from
        // logical 0 to logical 1

        if (activeLevel == HIGH)
        {
            // High voltage == logical 1
            
            if (mode == LEVEL_UNDEFINED || mode == LEVEL_RISING)
            {
                retVal = LEVEL_RISING;
            }
            else if (mode == LEVEL_FALLING)
            {
                retVal = LEVEL_FALLING;
            }
            else
            {
                retVal = LEVEL_RISING_AND_FALLING;
            }
        }
        else
        {
            // Low voltage == logical 1
            
            if (mode == LEVEL_RISING)
            {
                retVal = LEVEL_FALLING;
            }
            else if (mode == LEVEL_UNDEFINED || mode == LEVEL_FALLING)
            {
                retVal = LEVEL_RISING;
            }
            else
            {
                retVal = LEVEL_RISING_AND_FALLING;
            }
        }
        
        return retVal;
    }
    
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