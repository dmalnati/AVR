#include "Evm.h"
#include "IdleTimeEventHandler.h"


//////////////////////////////////////////////////////////////////////
//
// Idle Events
//
//////////////////////////////////////////////////////////////////////

void IdleTimeEventHandler::RegisterForIdleTimeEvent()
{
    // Don't allow yourself to be scheduled more than once
    DeRegisterForIdleTimeEvent();
    
    Evm::GetInstance().RegisterIdleTimeEventHandler(this);
}

void IdleTimeEventHandler::DeRegisterForIdleTimeEvent()
{
    Evm::GetInstance().DeRegisterIdleTimeEventHandler(this);
}


