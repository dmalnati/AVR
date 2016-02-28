#ifndef __TIMED_EVENT_HANDLER_H__
#define __TIMED_EVENT_HANDLER_H__


#include <stdint.h>


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
// Object Wrappers
//
//////////////////////////////////////////////////////////////////////

template <typename T>
class TimedEventHandlerDelegate
: public TimedEventHandler
{
    typedef void (T::*MemberCallbackFn)();
    
public:
    void SetCallback(T *obj, MemberCallbackFn func)
    {
        obj_  = obj;
        func_ = func;
    }

private:
    virtual void OnTimedEvent()
    {
        if (obj_ && func_) { ((*obj_).*func_)(); }
    }

    T                *obj_;
    MemberCallbackFn  func_;
};



#endif  // __TIMED_EVENT_HANDLER_H__










