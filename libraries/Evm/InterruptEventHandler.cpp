#include "InterruptEventHandler.h"
#include "ISR.h"


//////////////////////////////////////////////////////////////////////
//
// Interrupt Events
//
//////////////////////////////////////////////////////////////////////


void InterruptEventHandler::RegisterForInterruptEvent()
{
    // Don't allow yourself to be registered more than once
    DeRegisterForInterruptEvent();
    
    ISR_RegisterForInterruptEvent(this);
}

void InterruptEventHandler::DeRegisterForInterruptEvent()
{
    ISR_DeRegisterForInterruptEvent(this);
}






