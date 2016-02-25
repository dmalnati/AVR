#include <util/atomic.h>

#include "PCIntEventHandler.h"


uint8_t PCIntEventHandler::
RegisterForPCIntEvent(uint8_t pin, MODE mode)
{
    uint8_t retVal = 0;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        // Don't allow double registration
        DeRegisterForPCIntEvent();
        
        if (mode != MODE::MODE_UNDEFINED)
        {
            pin_  = pin;
            mode_ = mode;
            
            retVal = Ivm::GetInstance().RegisterPCIntEventHandler(this);
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
        retVal = Ivm::GetInstance().DeRegisterPCIntEventHandler(this);
        
        pin_  = -1;
        mode_ = MODE::MODE_UNDEFINED;
    }
    
    return retVal;
}
    
    
    
    
    