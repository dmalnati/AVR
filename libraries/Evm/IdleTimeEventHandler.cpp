#include "Evm.h"
#include "IdleTimeEventHandler.h"

//////////////////////////////////////////////////////////////////////
//
// Idle Events
//
//////////////////////////////////////////////////////////////////////

uint8_t IdleTimeEventHandler::
RegisterForIdleTimeEvent()
{
    // Don't allow yourself to be scheduled more than once
    DeRegisterForIdleTimeEvent();
    
    return Evm::GetInstance().RegisterIdleTimeEventHandler(this);
}

uint8_t IdleTimeEventHandler::
DeRegisterForIdleTimeEvent()
{
    return Evm::GetInstance().DeRegisterIdleTimeEventHandler(this);
}


