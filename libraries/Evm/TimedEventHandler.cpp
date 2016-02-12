#include "Evm.h"
#include "TimedEventHandler.h"


//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////

void TimedEventHandler::RegisterForTimedEvent(uint32_t timeout)
{
    // Don't allow yourself to be scheduled more than once.
    // Cache whether this is an interval callback since that
    // gets reset during cancel.
    uint8_t isIntervalCache = isInterval_;
    DeRegisterForTimedEvent();
    isInterval_ = isIntervalCache;
    
    Evm::GetInstance().RegisterTimedEventHandler(this, timeout);
}

void TimedEventHandler::RegisterForTimedEventInterval(uint32_t timeout)
{
    isInterval_ = 1;
    
    RegisterForTimedEvent(timeout);
}

void TimedEventHandler::DeRegisterForTimedEvent()
{
    Evm::GetInstance().DeRegisterTimedEventHandler(this);
    
    // make sure this isn't re-scheduled if interval
    isInterval_ = 0;
}











