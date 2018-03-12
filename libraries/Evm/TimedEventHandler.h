#ifndef __TIMED_EVENT_HANDLER_H__
#define __TIMED_EVENT_HANDLER_H__


#include <stdint.h>

#include "Function.h"


// Forward declaration
template <uint8_t, uint8_t, uint8_t>
class EvmActual;


//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////

class TimedEventHandler
{
    template <uint8_t, uint8_t, uint8_t>
    friend class EvmActual;
    
public:
    TimedEventHandler() : isInterval_(0) { }
    virtual ~TimedEventHandler() { DeRegisterForTimedEvent(); }

    uint8_t RegisterForTimedEvent(uint32_t timeout);
    uint8_t RegisterForTimedEventInterval(uint32_t timeout);
    uint8_t RegisterForTimedEventInterval(uint32_t timeout, uint32_t firstTimeout);
    uint8_t DeRegisterForTimedEvent();
    
    virtual void OnTimedEvent() = 0;

private:
    // Evm uses these for state keeping
    uint32_t timeQueued_;
    uint32_t timeout_;
    uint8_t  isInterval_;
    uint32_t intervalTimeout_;
};


//////////////////////////////////////////////////////////////////////
//
// Helpers
//
//////////////////////////////////////////////////////////////////////


class TimedEventHandlerDelegate
: public TimedEventHandler
{
public:
    TimedEventHandlerDelegate()
    {
        // Nothing to do
    }
    
    TimedEventHandlerDelegate(function<void()> cbFn)
    {
        SetCallback(cbFn);
    }

    void SetCallback(function<void()> cbFn)
    {
        cbFn_ = cbFn;
    }
    
    function<void()> GetCallback()
    {
        return cbFn_;
    }
    
    void operator()()
    {
        GetCallback()();
    }

private:
    virtual void OnTimedEvent()
    {
        cbFn_();
    }

    function<void()> cbFn_;
};



#endif  // __TIMED_EVENT_HANDLER_H__










