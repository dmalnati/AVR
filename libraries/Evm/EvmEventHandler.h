#ifndef __EVM_CALLBACK_H__
#define __EVM_CALLBACK_H__


#include <stdint.h>


class Evm;


//////////////////////////////////////////////////////////////////////
//
// Idle Events
//
//////////////////////////////////////////////////////////////////////

class IdleTimeEventHandler
{
public:
    virtual ~IdleTimeEventHandler() { DeRegisterForIdleTimeEvent(); }
    
    void RegisterForIdleTimeEvent();
    void DeRegisterForIdleTimeEvent();
    
    virtual void OnIdleTimeEvent() = 0;
};


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

    void RegisterForTimedEvent(uint32_t duration);
    void RegisterForTimedEventInterval(uint32_t duration);
    void DeRegisterForTimedEvent();
    
    virtual void OnTimedEvent() = 0;

private:
    // Evm uses these for state keeping
    uint32_t timeQueued_;
    uint32_t duration_;
    uint8_t  isInterval_;
};


//////////////////////////////////////////////////////////////////////
//
// Function Wrappers
//
//////////////////////////////////////////////////////////////////////

class IdleTimeEventHandlerFnWrapper : public IdleTimeEventHandler
{
    typedef void (*CallbackFn)(void *userData);
    
public:
    IdleTimeEventHandlerFnWrapper(CallbackFn fn, void *userData)
    : fn_(fn)
    , userData_(userData)
    {
        // nothing to do
    }
    void OnIdleTimeEvent() { fn_(userData_); }

private:
    CallbackFn  fn_;
    void       *userData_;
};


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





#endif  // __EVM_CALLBACK_H__