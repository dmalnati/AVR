#include "Evm.h"
#include "TimedEventHandler.h"


//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////

uint8_t TimedEventHandler::
RegisterForTimedEvent(uint32_t timeout)
{
    // Don't allow yourself to be scheduled more than once.
    // Cache whether this is an interval callback since that
    // gets reset during cancel.
    uint8_t isIntervalCache = isInterval_;
    DeRegisterForTimedEvent();
    isInterval_ = isIntervalCache;
    
    return Evm::GetInstance().RegisterTimedEventHandler(this, timeout);
}

uint8_t TimedEventHandler::
RegisterForTimedEventInterval(uint32_t timeout)
{
    isInterval_ = 1;
    
    return RegisterForTimedEvent(timeout);
}

uint8_t TimedEventHandler::
DeRegisterForTimedEvent()
{
    uint8_t retVal = Evm::GetInstance().DeRegisterTimedEventHandler(this);
    
    // make sure this isn't re-scheduled if interval
    isInterval_ = 0;
    
    return retVal;
}











