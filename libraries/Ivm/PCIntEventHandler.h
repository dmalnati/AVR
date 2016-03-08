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
    : pin_(0)
    , mode_(MODE::MODE_UNDEFINED)
    , logicLevel_(0)
    {
        // Nothing to do
    }
    
    virtual ~PCIntEventHandler()
    {
        DeRegisterForPCIntEvent();
    }

    uint8_t RegisterForPCIntEvent(uint8_t pin, MODE mode);
    uint8_t DeRegisterForPCIntEvent();
    
    uint8_t GetPin()        const { return pin_;        }
    MODE    GetMode()       const { return mode_;       }
    uint8_t GetLogicLevel() const { return logicLevel_; }
    
    
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
    uint8_t logicLevel_;
};








#endif  // __PCINT_EVENT_HANDLER_H__








