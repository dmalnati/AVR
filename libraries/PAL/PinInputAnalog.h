#ifndef __PIN_INPUT_ANALOG_H__
#define __PIN_INPUT_ANALOG_H__


#include "PAL.h"
#include "Function.h"
#include "TimedEventHandler.h"


class PinInputAnalog
: private TimedEventHandler
{
public:
    static const uint8_t C_IDLE  = 0;
    static const uint8_t C_TIMED = 1;
    static const uint8_t C_INTER = 0;
    
private:
    static const uint32_t DEFAULT_POLL_PERIOD_MS = 100;
    static const uint16_t DEFAULT_MINIMUM_CHANGE = 15;
    
public:
    PinInputAnalog(uint8_t  pin,
                   uint32_t pollPeriodMs = DEFAULT_POLL_PERIOD_MS)
    : pin_(pin)
    , pollPeriodMs_(pollPeriodMs)
    , minimumChange_(-1)
    , valLast_(-1)
    {
        SetMinimumChange(DEFAULT_MINIMUM_CHANGE);
        
        fnAnalogRead_ = [](uint8_t pin) {
            return PAL.AnalogRead(pin);
        };
    }
    
    void SetMinimumChange(uint16_t minimumChange)
    {
        minimumChange_ = minimumChange;
    }
    
    void SetCallback(function<void(uint16_t val)> &&cbFn)
    {
        cbFn_ = cbFn;
    }
    
    void SetAnalogReadFunction(function<uint16_t(uint8_t)> fnAnalogRead)
    {
        fnAnalogRead_ = fnAnalogRead;
    }
    
    uint8_t GetValue(uint8_t forceRefresh = 0)
    {
        uint8_t retVal = valLast_;
        
        if (forceRefresh || valLast_ == -1)
        {
            valLast_ = fnAnalogRead_(pin_);
            
            retVal = valLast_;
        }
        
        return retVal;
    }
    
    void Disable()
    {
        DeRegisterForTimedEvent();
    }
    
    void Enable()
    {
        RegisterForTimedEventInterval(pollPeriodMs_);
    }

private:
    virtual void OnTimedEvent()
    {
        uint16_t val = fnAnalogRead_(pin_);
        
        if (minimumChange_ != -1)
        {
            // Fire when:
            // - never fired before
            // - changed by the minimum
            // - hit either extreme of the spectrum of values
            if (valLast_ == -1                        ||
                abs(valLast_ - val) >= minimumChange_ ||
                (val == 1023 && valLast_ != 1023)     ||
                (val == 0    && valLast_ != 0))
            {
                cbFn_(val);
                
                valLast_ = val;
            }
        }
        else
        {
            cbFn_(val);
        }
    }

    uint8_t                      pin_;
    uint32_t                     pollPeriodMs_;
    int32_t                      minimumChange_;
    int32_t                      valLast_;
    function<void(uint16_t val)> cbFn_;
    function<uint16_t(uint8_t)>  fnAnalogRead_;
};




#endif  // __PIN_INPUT_ANALOG_H__