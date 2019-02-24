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
    uint8_t isRigidCache    = isRigid_;
    DeRegisterForTimedEvent();
    isInterval_ = isIntervalCache;
    isRigid_    = isRigidCache;
    
    return Evm::GetInstance().RegisterTimedEventHandler(this, timeout);
}

uint8_t TimedEventHandler::
RegisterForTimedEventInterval(uint32_t timeout)
{
    isInterval_ = 1;
    isRigid_    = 0;
    
    intervalTimeout_ = timeout;
    
    return RegisterForTimedEvent(timeout);
}

uint8_t TimedEventHandler::
RegisterForTimedEventInterval(uint32_t timeout, uint32_t firstTimeout)
{
    isInterval_ = 1;
    isRigid_    = 0;
    
    intervalTimeout_ = timeout;
    
    return RegisterForTimedEvent(firstTimeout);
}

uint8_t TimedEventHandler::
RegisterForTimedEventIntervalRigid(uint32_t timeout)
{
    uint8_t retVal = RegisterForTimedEventInterval(timeout);
    
    isRigid_ = 1;
    
    return retVal;
}

uint8_t TimedEventHandler::
RegisterForTimedEventIntervalRigid(uint32_t timeout, uint32_t firstTimeout)
{
    uint8_t retVal = RegisterForTimedEventInterval(timeout, firstTimeout);
    
    isRigid_ = 1;
    
    return retVal;
}

uint8_t TimedEventHandler::
DeRegisterForTimedEvent()
{
    uint8_t retVal = Evm::GetInstance().DeRegisterTimedEventHandler(this);
    
    // make sure this isn't re-scheduled if interval and you attempt to
    // deregister yourself from within your own callback.
    isInterval_ = 0;
    isRigid_    = 0;
    
    return retVal;
}

uint8_t TimedEventHandler::
IsRegistered()
{
    uint8_t retVal = Evm::GetInstance().IsRegisteredTimedEventHandler(this);
    
    return retVal;
}

uint32_t TimedEventHandler::
GetTimeQueued()
{
    return timeQueued_;
}











