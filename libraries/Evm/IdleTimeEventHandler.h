#ifndef __IDLE_TIME_EVENT_HANDLER__
#define __IDLE_TIME_EVENT_HANDLER__


#include <stdint.h>


// Forward declaration
template <uint8_t COUNT_IDLE_TIME_EVENT_HANDLER,
      uint8_t COUNT_TIMED_EVENT_HANDLER,
      uint8_t COUNT_INTERRUPT_EVENT_HANDLER>
class EvmActual;


//////////////////////////////////////////////////////////////////////
//
// Idle Events
//
//////////////////////////////////////////////////////////////////////

class IdleTimeEventHandler
{
    template <uint8_t COUNT_IDLE_TIME_EVENT_HANDLER,
          uint8_t COUNT_TIMED_EVENT_HANDLER,
          uint8_t COUNT_INTERRUPT_EVENT_HANDLER>
    friend class EvmActual;
    
    
public:
    virtual ~IdleTimeEventHandler() { DeRegisterForIdleTimeEvent(); }
    
    uint8_t RegisterForIdleTimeEvent();
    uint8_t DeRegisterForIdleTimeEvent();
    
    virtual void OnIdleTimeEvent() = 0;
    
private:
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
    : IdleTimeEventHandler()
    , fn_(fn)
    , userData_(userData)
    {
        // nothing to do
    }
    void OnIdleTimeEvent() { fn_(userData_); }

private:
    CallbackFn  fn_;
    void       *userData_;
};




#endif  // __IDLE_TIME_EVENT_HANDLER__










