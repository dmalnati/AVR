#include <util/atomic.h>

#include "PCIntEventHandler.h"


uint8_t PCIntEventHandler::
RegisterForPCIntEvent()
{
    uint8_t retVal;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        // Don't allow double registration
        DeRegisterForPCIntEvent();
        
        retVal = Ivm::GetInstance().RegisterPCIntEventHandler(this);
    }
    
    return retVal;
}

uint8_t PCIntEventHandler::
DeRegisterForPCIntEvent()
{
    return Ivm::GetInstance().DeRegisterPCIntEventHandler(this);
}
    
    
    
    
    