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
    Evm::GetInstance().SetIdleCallback(this);
}


//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////

void TimedCallback::Schedule(uint32_t duration)
{
    Evm::GetInstance().SetTimeout(duration, this);
}

void TimedCallback::ScheduleInterval(uint32_t duration)
{
    isInterval_ = 1;
    
    Schedule(duration);
}