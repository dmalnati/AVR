#ifndef __EVM_EVENT_HANDLER_UTILS_H__
#define __EVM_EVENT_HANDLER_UTILS_H__


#include <PAL.h>
#include <TimedEventHandler.h>


/*
 * Misc classes that I keep implementing everywhere.
 *
 * Not part of the core libs
 *
 */



class TimedPinToggler : public TimedEventHandler
{
public:
    TimedPinToggler(uint8_t pin) : pin_(pin) { pinMode(pin_, OUTPUT); }
    
    void OnTimedEvent()
    {
        PAL.DigitalWrite(pin_, HIGH);
        PAL.DigitalWrite(pin_, LOW);
    }

private:
    uint8_t pin_;
};





#endif // __EVM_EVENT_HANDLER_UTILS_H__