#include "InterruptEventHandler.h"
#include "ISR.h"


//////////////////////////////////////////////////////////////////////
//
// Interrupt Events
//
//////////////////////////////////////////////////////////////////////


void InterruptEventHandler::RegisterForInterruptEvent()
{
    // No need to prevent double-registration.
    // This is already handled elsewhere in the ISR routines.
    
    ISR_RegisterForInterruptEvent(this);
}

void InterruptEventHandler::DeRegisterForInterruptEvent()
{
    ISR_DeRegisterForInterruptEvent(this);
}






