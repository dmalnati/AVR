#include <util/atomic.h>

#include "PCIntEventHandler.h"


uint8_t PCIntEventHandler::
RegisterForPCIntEvent(uint8_t pin, MODE mode)
{
    uint8_t retVal = 0;
    
    // commenting out or not this next line leaves the LED on or off at
    // startup.  must be some kind of static init sequencing issue.
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        // Don't allow double registration
        DeRegisterForPCIntEvent();
        
        if (mode != MODE::MODE_UNDEFINED)
        {
            pin_  = pin;
            mode_ = mode;
            
            retVal = Ivm::RegisterPCIntEventHandler(this);
        }
    }
    
    return retVal;
}

uint8_t PCIntEventHandler::
DeRegisterForPCIntEvent()
{
    uint8_t retVal;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        retVal = Ivm::DeRegisterPCIntEventHandler(this);
    }
    
    return retVal;
}
    
    
    
    
    