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
             uint8_t activeLevel = HIGH,
             uint8_t mode        = LEVEL_UNDEFINED)
    : logicLevelActual_(PAL.DigitalRead(pin))
    , activeLevel_(activeLevel)
    {
        if (mode == LEVEL_UNDEFINED)
        {
            // assume logical HIGH
            if (activeLevel_ == HIGH)
            {
                RegisterForInterruptEvent(pin, LEVEL_RISING);
            }
            else
            {
                RegisterForInterruptEvent(pin, LEVEL_FALLING);
            }
        }
        else if (mode == LEVEL_RISING)
        {
            RegisterForInterruptEvent(pin, LEVEL_RISING);
        }
        else if (mode == LEVEL_FALLING)
        {
            RegisterForInterruptEvent(pin, LEVEL_FALLING);
        }
        else
        {
            RegisterForInterruptEvent(pin, LEVEL_RISING_AND_FALLING);
        }
    }
    
    void SetCallback(function<void(uint8_t logicLevel)> &&cbFn)
    {
        cbFn_ = cbFn;
    }
    
    uint8_t GetLogicLevel()
    {
        uint8_t retVal = logicLevelActual_;
        
        if (activeLevel_ == LOW)
        {
            retVal = !retVal;
        }
        
        return retVal;
    }

private:
    virtual void OnInterruptEvent(uint8_t logicLevel)
    {
        logicLevelActual_ = logicLevel;
        
        cbFn_(GetLogicLevel());
    }

    uint8_t                 logicLevelActual_;
    uint8_t                 activeLevel_;
    function<void(uint8_t)> cbFn_;
};


#endif  // __PIN_INPUT_H__