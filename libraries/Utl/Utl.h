#ifndef __UTL_H__
#define __UTL_H__


#include "PAL.h"
#include "IdleTimeEventHandler.h"
#include "TimedEventHandler.h"
#include "IdleTimeHiResTimedEventHandler.h"




extern void PinToggle(uint8_t pin, uint16_t delayMs = 500);


class TimedPinToggler : public TimedEventHandler
{
public:
    TimedPinToggler(uint8_t pin) : pin_(pin)
    {
        PAL.PinMode(pin_, OUTPUT);
    }
    
    void OnTimedEvent()
    {
        PAL.DigitalWrite(pin_, HIGH);
        PAL.DigitalWrite(pin_, LOW);
    }

private:
    uint8_t pin_;
};


class TimedPinTogglerHiRes
: public IdleTimeHiResTimedEventHandler
{
public:
    TimedPinTogglerHiRes(uint8_t pin) : pin_(pin)
    {
        PAL.PinMode(pin_, OUTPUT);
    }
    
    void OnIdleTimeHiResTimedEvent()
    {
        PAL.DigitalWrite(pin_, HIGH);
        PAL.DigitalWrite(pin_, LOW);
    }
    
private:
    uint8_t pin_;
};


class IdlePinToggler : public IdleTimeEventHandler
{
public:
    IdlePinToggler(uint8_t pin) : pin_(pin)
    {
        PAL.PinMode(pin_, OUTPUT);
    }
    
    virtual void OnIdleTimeEvent()
    {
        PAL.DigitalWrite(pin_, HIGH);
        PAL.DigitalWrite(pin_, LOW);
    }
    
private:
    uint8_t pin_;
};



class DestructorToken
{
public:
    DestructorToken(uint8_t pin) : pin_(pin) { }
    ~DestructorToken()
    {
        PinToggle(pin_);
    }
    uint8_t pin_;
};





#endif // __UTL_H__









