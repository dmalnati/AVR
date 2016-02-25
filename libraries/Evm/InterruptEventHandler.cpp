#include "Evm.h"
#include "InterruptEventHandler.h"


uint8_t InterruptEventHandler::
RegisterForInterruptEvent(uint8_t pin, uint8_t mode)
{
    uint8_t retVal;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        // Don't allow double registration
        DeRegisterForInterruptEvent();
        
        retVal =
            RegisterForPCIntEvent(pin, ConvertToPCIntEventHandlerMode(mode));
    }
    
    return retVal;
}

uint8_t InterruptEventHandler::
DeRegisterForInterruptEvent()
{
    uint8_t retVal;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        uint8_t retVal1 = DeRegisterForPCIntEvent();
        
        uint8_t retVal2 =
            Evm::GetInstance().DeRegisterInterruptEventHandler(this);
        
        retVal = retVal1 && retVal2;
    }
    
    return retVal;
}


void InterruptEventHandler::
OnPCIntEvent(uint8_t logicLevel)
{
    logicLevel_ = logicLevel;
    
    Evm::GetInstance().RegisterInterruptEventHandler(this);
}


