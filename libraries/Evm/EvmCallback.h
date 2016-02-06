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

























/*
//////////////////////////////////////////////////////////////////////
//
// Simple Callback interface, and wrapper for pure functions
//
//////////////////////////////////////////////////////////////////////

class Callback
{
public:
    Callback() { }
    ~Callback() { }
    virtual void OnCallback() = 0;
};

template<CallbackFn fn, void *userData>
class CallbackFnWrapper : public Callback
{
public:
    CallbackFnWrapper() : fn_(fn), userData_(userData) { }
    ~CallbackFnWrapper() { }
    
    virtual void OnCallback() { fn_(userData_); }
private:
    CallbackFn  fn_;
    void       *userData_;
};


//////////////////////////////////////////////////////////////////////
//
// Simple Timed Callback interface, and wrapper for pure functions
//
//////////////////////////////////////////////////////////////////////

class TimedCallback : public Callback
{
    friend class Evm;

private:
    uint32_t timeQueued_;
    uint32_t duration_;
};

template<CallbackFn fn, void *userData>
class TimedCallbackFnWrapper : public TimedCallback
{
public:
    TimedCallbackFnWrapper() : fn_(fn), userData_(userData) { }
    ~TimedCallbackFnWrapper() { }
    
    virtual void OnCallback() { fn_(userData_); }
private:
    CallbackFn  fn_;
    void       *userData_;
};

*/

//////////////////////////////////////////////////////////////////////
//
// Callback system
// - Only way to schedule Evm work is through these classes
//
//////////////////////////////////////////////////////////////////////





















#endif  // __EVM_CALLBACK_H__