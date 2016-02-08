#include "Evm.h"
#include "EvmCallback.h"

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

void IdleCallback::Start()
{
    // Don't allow yourself to be scheduled more than once
    Stop();
    Evm::GetInstance().SetIdleCallback(this);
}

void IdleCallback::Stop()
{
    Evm::GetInstance().CancelIdleCallback(this);
}


//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////

void TimedCallback::Schedule(uint32_t duration)
{
    // Don't allow yourself to be scheduled more than once
    uint8_t isIntervalCache = isInterval_;
    Cancel();
    isInterval_ = isIntervalCache;
    
    Evm::GetInstance().SetTimeout(duration, this);
}

void TimedCallback::ScheduleInterval(uint32_t duration)
{
    isInterval_ = 1;
    
    Schedule(duration);
}

void TimedCallback::Cancel()
{
    Evm::GetInstance().CancelTimeout(this);
    
    // make sure this isn't re-scheduled if interval
    isInterval_ = 0;
}











