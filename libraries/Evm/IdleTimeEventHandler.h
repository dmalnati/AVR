#ifndef __IDLE_TIME_EVENT_HANDLER__
#define __IDLE_TIME_EVENT_HANDLER__


//////////////////////////////////////////////////////////////////////
//
// Idle Events
//
//////////////////////////////////////////////////////////////////////

template <typename EvmT>
class IdleTimeEventHandler
{
public:
    IdleTimeEventHandler(EvmT &evm) : evm_(evm) { }
    virtual ~IdleTimeEventHandler() { DeRegisterForIdleTimeEvent(); }
    
    uint8_t RegisterForIdleTimeEvent();
    uint8_t DeRegisterForIdleTimeEvent();
    
    virtual void OnIdleTimeEvent() = 0;
    
private:
    EvmT &evm_;
};


#include "IdleTimeEventHandler.hpp"


//////////////////////////////////////////////////////////////////////
//
// Function Wrappers
//
//////////////////////////////////////////////////////////////////////

template <typename EvmT>
class IdleTimeEventHandlerFnWrapper : public IdleTimeEventHandler<EvmT>
{
    typedef void (*CallbackFn)(void *userData);
    
public:
    IdleTimeEventHandlerFnWrapper(EvmT &evm, CallbackFn fn, void *userData)
    : IdleTimeEventHandler<EvmT>(evm)
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










