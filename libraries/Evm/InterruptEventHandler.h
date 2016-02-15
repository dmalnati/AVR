#ifndef __INTERRUPT_EVENT_HANDLER_H__
#define __INTERRUPT_EVENT_HANDLER_H__


#include <stdint.h>


//////////////////////////////////////////////////////////////////////
//
// Interrupt Events
//
//////////////////////////////////////////////////////////////////////

class InterruptEventHandler
{
public:
    enum class MODE : uint8_t { MODE_FALLING = 0, MODE_RISING = 1 };
    
    InterruptEventHandler(uint8_t pin, MODE mode = MODE::MODE_RISING)
    : pin_(pin)
    , mode_(mode)
    {
        // nothing to do
    }
    ~InterruptEventHandler() { DeRegisterForInterruptEvent(); }
    
    uint8_t GetPin() const { return pin_; }
    MODE GetMode() const { return mode_; }
    
    void RegisterForInterruptEvent();
    void DeRegisterForInterruptEvent();
    
    virtual void OnInterruptEvent() = 0;
    
private:
    uint8_t pin_;
    MODE mode_;
};



//////////////////////////////////////////////////////////////////////
//
// Object Wrappers
//
//////////////////////////////////////////////////////////////////////

template <typename T>
class InterruptEventHandlerObjectWrapper
: public InterruptEventHandler
{
    typedef void (T::*MemberCallbackFn)();
    
public:
    InterruptEventHandlerObjectWrapper(
        uint8_t           pin,
        T                *obj,
        MemberCallbackFn  func,
        MODE              mode = MODE::MODE_RISING)
    : InterruptEventHandler(pin, mode)
    , obj_(obj)
    , func_(func)
    {
        // nothing to do
    }

private:
    virtual void OnInterruptEvent()
    {
        ((*obj_).*func_)();
    }

    T                *obj_;
    MemberCallbackFn  func_;
};

template <typename T>
InterruptEventHandlerObjectWrapper<T> *
MapButNotStartInterrupt(
    uint8_t                      pin,
    T                           *obj,
    void                         (T::*cbFn)(),
    InterruptEventHandler::MODE  mode = InterruptEventHandler::MODE::MODE_RISING)
{
    InterruptEventHandlerObjectWrapper<T> * iehow =
        new InterruptEventHandlerObjectWrapper<T>(pin, obj, cbFn, mode);
    
    return iehow;
}

template <typename T>
InterruptEventHandlerObjectWrapper<T> *
MapAndStartInterrupt(
    uint8_t                      pin,
    T                           *obj,
    void                         (T::*cbFn)(),
    InterruptEventHandler::MODE  mode = InterruptEventHandler::MODE::MODE_RISING)
{
    InterruptEventHandlerObjectWrapper<T> * iehow =
        MapButNotStartInterrupt(pin, obj, cbFn, mode);

    iehow->RegisterForInterruptEvent();
    
    return iehow;
}



//////////////////////////////////////////////////////////////////////
//
// Function Wrappers
//
//////////////////////////////////////////////////////////////////////

class InterruptEventHandlerFnWrapper : public InterruptEventHandler
{
    typedef void (*CallbackFn)(void *userData);
    
public:
    InterruptEventHandlerFnWrapper(uint8_t     pin,
                                   MODE        mode,
                                   CallbackFn  fn,
                                   void       *userData)
    : InterruptEventHandler(pin, mode)
    , fn_(fn)
    , userData_(userData)
    {
        // nothing to do
    }
    void OnInterruptEvent() { fn_(userData_); }

private:
    CallbackFn  fn_;
    void       *userData_;
};



#endif  // __INTERRUPT_EVENT_HANDLER_H__





















