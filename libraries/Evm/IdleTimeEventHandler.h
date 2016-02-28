#ifndef __IDLE_TIME_EVENT_HANDLER__
#define __IDLE_TIME_EVENT_HANDLER__


#include <stdint.h>


// Forward declaration
template <uint8_t, uint8_t, uint8_t>
class EvmActual;


//////////////////////////////////////////////////////////////////////
//
// Idle Events
//
//////////////////////////////////////////////////////////////////////

class IdleTimeEventHandler
{
    template <uint8_t, uint8_t, uint8_t>
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

class IdleTimeEventHandlerFnWrapper
: public IdleTimeEventHandler
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




#endif  // __IDLE_TIME_EVENT_HANDLER__










