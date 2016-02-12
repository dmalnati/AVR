#ifndef __TIMED_EVENT_HANDLER_H__
#define __TIMED_EVENT_HANDLER_H__


#include <stdint.h>


//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////

class TimedEventHandler
{
    friend class Evm;
    
public:
    TimedEventHandler() : isInterval_(0) { }
    virtual ~TimedEventHandler() { DeRegisterForTimedEvent(); }

    void RegisterForTimedEvent(uint32_t timeout);
    void RegisterForTimedEventInterval(uint32_t timeout);
    void DeRegisterForTimedEvent();
    
    virtual void OnTimedEvent() = 0;

private:
    // Evm uses these for state keeping
    uint32_t timeQueued_;
    uint32_t timeout_;
    uint8_t  isInterval_;
};


//////////////////////////////////////////////////////////////////////
//
// Function Wrappers
//
//////////////////////////////////////////////////////////////////////

class TimedEventHandlerFnWrapper : public TimedEventHandler
{
    typedef void (*CallbackFn)(void *userData);
    
public:
    TimedEventHandlerFnWrapper(CallbackFn fn, void *userData)
    : fn_(fn)
    , userData_(userData)
    {
        // nothing to do
    }
    void OnTimedEvent() { fn_(userData_); }

private:
    CallbackFn  fn_;
    void       *userData_;
};





#endif  // __TIMED_EVENT_HANDLER_H__










