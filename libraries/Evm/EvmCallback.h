#ifndef __EVM_CALLBACK_H__
#define __EVM_CALLBACK_H__


#include <stdint.h>


class Evm;


//////////////////////////////////////////////////////////////////////
//
// Basic Interface
//
//////////////////////////////////////////////////////////////////////

class Callback
{
public:
    typedef void (*CallbackFn)(void *userData);

    virtual void OnCallback() = 0;
};


//////////////////////////////////////////////////////////////////////
//
// Idle Events
//
//////////////////////////////////////////////////////////////////////

class IdleCallback : public Callback
{
public:
    void Start();
    void Stop();
};


//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////

class TimedCallback : public Callback
{
    friend class Evm;
    
public:
    TimedCallback() : isInterval_(0) { }

    void Schedule(uint32_t duration);
    void ScheduleInterval(uint32_t duration);
    void Cancel();

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

class IdleCallbackFnWrapper : public IdleCallback
{
public:
    IdleCallbackFnWrapper(CallbackFn fn, void *userData)
    : fn_(fn)
    , userData_(userData)
    {
        // nothing to do
    }
    void OnCallback() { fn_(userData_); }

private:
    CallbackFn  fn_;
    void       *userData_;
};


class TimedCallbackFnWrapper : public TimedCallback
{
public:
    TimedCallbackFnWrapper(CallbackFn fn, void *userData)
    : fn_(fn)
    , userData_(userData)
    {
        // nothing to do
    }
    void OnCallback() { fn_(userData_); }

private:
    CallbackFn  fn_;
    void       *userData_;
};





#endif  // __EVM_CALLBACK_H__