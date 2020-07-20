#ifndef __PCINT_EVENT_HANDLER_H__
#define __PCINT_EVENT_HANDLER_H__


#include "Ivm.h"


class PCIntEventHandler
{
public:
    enum class MODE : uint8_t { MODE_UNDEFINED,
                                MODE_FALLING,
                                MODE_RISING,
                                MODE_RISING_AND_FALLING };

    PCIntEventHandler()
    : PCIntEventHandler(0, MODE::MODE_UNDEFINED)
    {
        // Nothing to do
    }

    PCIntEventHandler(uint8_t pin, MODE mode)
    : pin_(pin)
    , mode_(mode)
    , timeUsEvent_(0)
    {
        // Nothing to do
    }
    
    virtual ~PCIntEventHandler()
    {
        DeRegisterForPCIntEvent();
    }

    uint8_t RegisterForPCIntEvent(uint8_t pin, MODE mode);
    uint8_t RegisterForPCIntEvent()
    {
        return RegisterForPCIntEvent(pin_, mode_);
    }
    uint8_t DeRegisterForPCIntEvent();
    
    uint8_t GetPin()        const { return pin_;        }
    MODE    GetMode()       const { return mode_;       }
    
    void     SetEventTimeUs(uint32_t timeUsEvent) { timeUsEvent_ = timeUsEvent; }
    uint32_t GetEventTimeUs() const { return timeUsEvent_; }
    
    
    //
    // This function is being called from an ISR and is inherently dangerous
    // if it changes any data which is being looked at in other locations.
    //
    // Care must be taken anywhere this code leads to prevent the firing of
    // ISRs while modifying structures which this callback also leads to.
    //
    // Interrupts should not be enabled along any code path this leads to.
    //
    virtual void OnPCIntEvent(uint8_t logicLevel) = 0;
    
private:
    
    uint8_t pin_;
    MODE    mode_;

    volatile uint32_t timeUsEvent_;
};



//////////////////////////////////////////////////////////////////////
//
// Helpers
//
//////////////////////////////////////////////////////////////////////


class PCIntEventHandlerDelegate
: public PCIntEventHandler
{
public:
    PCIntEventHandlerDelegate()
    {
        // Nothing to do
    }

    PCIntEventHandlerDelegate(uint8_t pin, MODE mode)
    : PCIntEventHandler(pin, mode)
    {
        // Nothing to do
    }
    
    void SetCallback(function<void(uint8_t)> cbFn)
    {
        cbFn_ = cbFn;
    }
    
    function<void(uint8_t)> GetCallback()
    {
        return cbFn_;
    }
    

private:
    virtual void OnPCIntEvent(uint8_t logicLevel)
    {
        cbFn_(logicLevel);
    }

    function<void(uint8_t)> cbFn_;
};




#endif  // __PCINT_EVENT_HANDLER_H__








