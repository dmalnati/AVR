#ifndef __LED_BLINKER_H__
#define __LED_BLINKER_H__


#include "PAL.h"
#include "TimedEventHandler.h"


class LedBlinker
{
    static const uint32_t DEFAULT_DURATION_ON_MS  = 500;
    static const uint32_t DEFAULT_DURATION_OFF_MS = 500;
    
public:

    LedBlinker(uint8_t pin)
    : pin_(pin)
    , durationOnMs_(DEFAULT_DURATION_ON_MS)
    , durationOffMs_(DEFAULT_DURATION_OFF_MS)
    , onOff_(0)
    , running_(0)
    {
        PAL.PinMode(pin, OUTPUT);
        
        Off();
    }
    
    void SetDurationOffOn(uint32_t durationOffMs, uint32_t durationOnMs)
    {
        durationOffMs_ = durationOffMs;
        durationOnMs_  = durationOnMs;
        
        // start immediately if at runtime
        if (running_)
        {
            Start();
        }
    }
    
    void Start()
    {
        Stop();
        
        ted_.SetCallback([this](){ OnTimeout(); });
        ted_.RegisterForTimedEvent(durationOffMs_);
        
        running_ = 1;
    }
    
    void Stop()
    {
        ted_.DeRegisterForTimedEvent();
        
        running_ = 0;
        
        Off();
    }
    
    void On()
    {
        PAL.DigitalWrite(pin_, HIGH);
        
        onOff_ = 1;
    }
    
    void Off()
    {
        PAL.DigitalWrite(pin_, LOW);
        
        onOff_ = 0;
    }

private:

    void OnTimeout()
    {
        if (onOff_)
        {
            // was high, go low
            ted_.RegisterForTimedEvent(durationOffMs_);
            Off();
        }
        else
        {
            // was low, go high
            ted_.RegisterForTimedEvent(durationOnMs_);
            On();
        }
    }

    uint8_t  pin_;
    uint32_t durationOnMs_;
    uint32_t durationOffMs_;
    
    TimedEventHandlerDelegate ted_;
    
    uint8_t onOff_;
    
    uint8_t running_;
};


#endif  // __LED_BLINKER_H__









