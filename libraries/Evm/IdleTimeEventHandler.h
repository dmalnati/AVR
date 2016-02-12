#ifndef __IDLE_TIME_EVENT_HANDLER__
#define __IDLE_TIME_EVENT_HANDLER__


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




#endif  // __IDLE_TIME_EVENT_HANDLER__










