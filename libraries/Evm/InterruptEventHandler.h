#ifndef __INTERRUPT_EVENT_HANDLER_H__
#define __INTERRUPT_EVENT_HANDLER_H__


#include <Arduino.h>


#include <stdint.h>


//////////////////////////////////////////////////////////////////////
//
// Interrupt Events
//
//////////////////////////////////////////////////////////////////////

class InterruptEventHandler
{
public:
    InterruptEventHandler(uint8_t pin, uint8_t mode = RISING)
    : pin_(pin)
    , mode_(mode)
    {
        // nothing to do
    }
    ~InterruptEventHandler() { DeRegisterForInterruptEvent(); }
    
    uint8_t GetPin() const { return pin_; }
    uint8_t GetMode() const { return mode_; }
    
    void RegisterForInterruptEvent();
    void DeRegisterForInterruptEvent();
    
    virtual void OnInterruptEvent() = 0;
    
private:
    uint8_t pin_;
    uint8_t mode_;
};


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
                                   uint8_t     mode,
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





















