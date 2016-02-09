#include "Evm.h"
#include "EvmEventHandler.h"

/*
 * Have to keep some of this in a cc file so that the mutual-dependency
 * between the callbacks and Evm can get sorted out in the #includes
 * before the compiler sees the actual use of internals of either set of classes
 */




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


//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////

void TimedEventHandler::RegisterForTimedEvent(uint32_t duration)
{
    // Don't allow yourself to be scheduled more than once.
    // Cache whether this is an interval callback since that
    // gets reset during cancel.
    uint8_t isIntervalCache = isInterval_;
    DeRegisterForTimedEvent();
    isInterval_ = isIntervalCache;
    
    Evm::GetInstance().RegisterTimedEventHandler(this, duration);
}

void TimedEventHandler::RegisterForTimedEventInterval(uint32_t duration)
{
    isInterval_ = 1;
    
    RegisterForTimedEvent(duration);
}

void TimedEventHandler::DeRegisterForTimedEvent()
{
    Evm::GetInstance().DeRegisterTimedEventHandler(this);
    
    // make sure this isn't re-scheduled if interval
    isInterval_ = 0;
}











