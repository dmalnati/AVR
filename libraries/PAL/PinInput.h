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




// Simple implementation that assumes pulling low to indicate active, and
// only transitions to that level count for a callback.
// This is the most common use-case for button presses.
//
// Baked into the implementation is the assumption that the poll period
// is sufficient to deal with debouncing and genuine transitions.
class PinInputNoIvm
{
private:
    static const uint32_t POLL_PERIOD_MS = 50;

public:
    PinInputNoIvm(uint8_t pin,
                  uint8_t mode        = LEVEL_RISING,
                  uint8_t activeLevel = LOW)
    : pin_(pin)
    , activeLevel_(activeLevel)
    , mode_(mode)
    , levelLast_(0)
    {
        // Nothing to do
    }

    void SetCallback(function<void(uint8_t logicLevel)> cbFn)
    {
        cbFn_ = cbFn;
    }

    void Enable()
    {
        ted_.SetCallback([this](){
            OnTimeout();
        });

        ted_.RegisterForTimedEventInterval(POLL_PERIOD_MS, 0);

        PAL.PinMode(pin_, INPUT_PULLUP);
        levelLast_ = PAL.DigitalRead(pin_);
    }

    void Disable()
    {
        ted_.DeRegisterForTimedEvent();
    }

private:

    void OnTimeout()
    {
        PAL.PinMode(pin_, INPUT_PULLUP);

        uint8_t level = PAL.DigitalRead(pin_);

        if (level != levelLast_)
        {
            if (!level)
            {
                cbFn_(!level);
            }
        }

        levelLast_ = level;
    }


private:
    uint8_t pin_;
    uint8_t activeLevel_;
    uint8_t mode_;

    function<void(uint8_t logicLevel)> cbFn_;

    uint8_t levelLast_;

    TimedEventHandlerDelegate ted_;
};



#endif  // __PIN_INPUT_H__