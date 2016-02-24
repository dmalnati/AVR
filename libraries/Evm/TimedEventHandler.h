#ifndef __TIMED_EVENT_HANDLER_H__
#define __TIMED_EVENT_HANDLER_H__


#include <stdint.h>


// Forward declaration
template <uint8_t COUNT_IDLE_TIME_EVENT_HANDLER,
      uint8_t COUNT_TIMED_EVENT_HANDLER,
      uint8_t COUNT_INTERRUPT_EVENT_HANDLER>
class EvmActual;


//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////

class TimedEventHandler
{
    template <uint8_t COUNT_IDLE_TIME_EVENT_HANDLER,
          uint8_t COUNT_TIMED_EVENT_HANDLER,
          uint8_t COUNT_INTERRUPT_EVENT_HANDLER>
    friend class EvmActual;
    
    
public:
    TimedEventHandler() : isInterval_(0) { }
    virtual ~TimedEventHandler() { DeRegisterForTimedEvent(); }

    uint8_t RegisterForTimedEvent(uint32_t timeout);
    uint8_t RegisterForTimedEventInterval(uint32_t timeout);
    uint8_t DeRegisterForTimedEvent();
    
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
    : TimedEventHandler()
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










