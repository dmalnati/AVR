#ifndef __TIMED_EVENT_HANDLER_H__
#define __TIMED_EVENT_HANDLER_H__


#include <stdint.h>


//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////

template <typename EvmT>
class TimedEventHandler
{
    template
    <
        uint8_t COUNT_IDLE_TIME_EVENT_HANDLER,
        uint8_t COUNT_TIMED_EVENT_HANDLER,
        uint8_t COUNT_INTERRUPT_EVENT_HANDLER
    >
    friend class Evm;
    
public:
    TimedEventHandler(EvmT &evm) : evm_(evm), isInterval_(0) { }
    virtual ~TimedEventHandler() { DeRegisterForTimedEvent(); }

    uint8_t RegisterForTimedEvent(uint32_t timeout);
    uint8_t RegisterForTimedEventInterval(uint32_t timeout);
    uint8_t DeRegisterForTimedEvent();
    
    virtual void OnTimedEvent() = 0;

private:
    EvmT &evm_;
    
    // Evm uses these for state keeping
    uint32_t timeQueued_;
    uint32_t timeout_;
    uint8_t  isInterval_;
    
};


#include "TimedEventHandler.hpp"


//////////////////////////////////////////////////////////////////////
//
// Function Wrappers
//
//////////////////////////////////////////////////////////////////////

template <typename EvmT>
class TimedEventHandlerFnWrapper : public TimedEventHandler<EvmT>
{
    typedef void (*CallbackFn)(void *userData);
    
public:
    TimedEventHandlerFnWrapper(EvmT &sys, CallbackFn fn, void *userData)
    : TimedEventHandler<EvmT>(sys)
    , fn_(fn)
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










