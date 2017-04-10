#ifndef __IDLE_TIME_EVENT_HANDLER__
#define __IDLE_TIME_EVENT_HANDLER__


#include <stdint.h>

#include "Function.h"


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
// Helpers
//
//////////////////////////////////////////////////////////////////////


class IdleTimeEventHandlerDelegate
: public IdleTimeEventHandler
{
public:
    IdleTimeEventHandlerDelegate()
    {
        // Nothing to do
    }
    
    IdleTimeEventHandlerDelegate(function<void()> cbFn)
    {
        SetCallback(cbFn);
    }

    void SetCallback(function<void()> cbFn)
    {
        cbFn_ = cbFn;
    }

private:
    virtual void OnIdleTimeEvent()
    {
        cbFn_();
    }

    function<void()> cbFn_;
};


#endif  // __IDLE_TIME_EVENT_HANDLER__










