#ifndef __ISR_H__
#define __ISR_H__


#include "InterruptEventHandler.h"


extern void ISR_RegisterForInterruptEvent(InterruptEventHandler *ieh);
extern void ISR_DeRegisterForInterruptEvent(InterruptEventHandler *ieh);

 
 #endif // __ISR_H__