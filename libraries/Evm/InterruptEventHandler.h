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





















