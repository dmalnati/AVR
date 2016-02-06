#include <Arduino.h>

#include "Evm.h"


//////////////////////////////////////////////////////////////////////
//
// Idle Events
//
//////////////////////////////////////////////////////////////////////

void Evm::SetIdleCallback(Callback *cbo)
{
    idleEventList_.PushBack(cbo);
}

void Evm::CancelIdleCallback(Callback *cbo)
{
    idleEventList_.Remove(cbo);
}

void Evm::HandleIdleFunctions()
{
    // Have to deal with the fact that an idle event may cancel itself
    // or another idle event at any given time, as well as possibly add another,
    // anywhere in the list.
    // Strategy:
    // - At least don't blow over the bounds, check Size constantly
    // - ... good enough.
    //   - idle functions are not guaranteed to run in any order, or a given
    //     number of times relative to any other idle functions.  if we miss
    //     one as a result of a resize, tough.
    for (uint8_t i = 0; i < idleEventList_.Size(); ++i)
    {
        idleEventList_[i]->OnCallback();
    }
}



//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////

void Evm::SetTimeout(uint32_t duration, TimedCallback *cbo)
{
    SetTimeoutMs(duration, cbo);
}

void Evm::SetTimeoutMs(uint32_t duration, TimedCallback *cbo)
{
    // Note what time it is when timer requested
    uint32_t timeNow = millis();
    
    // Keep track of some useful state
    cbo->timeQueued_ = timeNow;
    cbo->duration_   = duration;

    // Queue it
    timedEventList_.PushSorted(cbo, CmpTimedCallback);
}

void Evm::CancelTimeout(TimedCallback *cbo)
{
    timedEventList_.Remove(cbo);
}

void Evm::HandleTimers()
{
    const uint8_t MAX_EVENTS_HANDLED = 4;
    
    if (timedEventList_.Size())
    {
        uint8_t        remainingEvents = MAX_EVENTS_HANDLED;
        bool           keepGoing       = true;
        TimedCallback *cbo             = NULL;
        
        do {
            cbo = timedEventList_.PeekFront();
            
            //uint32_t timeNow = micros();
            uint32_t timeNow = millis();
            
            // Check if the time since accepting event is gte than
            // the duration the event was supposed to wait for.
            // Handles wraparound this way.
            if ((timeNow - cbo->timeQueued_) >= cbo->duration_)
            {
                // drop this element from the list
                timedEventList_.PopFront();
                
                // invoke the callback
                cbo->OnCallback();
                
                // re-schedule if it is an interval timer
                if (cbo->isInterval_) { SetTimeout(cbo->duration_, cbo); }
                
                // only keep going if remaining quota of events remains
                --remainingEvents;
                keepGoing = remainingEvents;
            }
            else
            {
                keepGoing = false;
            }
        } while (keepGoing && timedEventList_.Size());
    }
}

void Evm::MainLoop()
{
    while (1)
    {
        HandleTimers();
        HandleIdleFunctions();
    }
}




//////////////////////////////////////////////////////////////////////
//
// Static Functions
//
//////////////////////////////////////////////////////////////////////


Evm &
Evm::GetInstance(uint8_t maxEventCapacity)
{
    static Evm *evm = NULL;
    
    if (!evm)
    {
        evm = new Evm(maxEventCapacity);
    }
    
    return *evm;
}

int8_t
Evm::CmpTimedCallback(TimedCallback *tc1, TimedCallback *tc2)
{
    int8_t retVal;
    
    uint32_t timeNow = millis();
    
    uint32_t expiryOne = (timeNow + tc1->timeQueued_) + tc1->duration_;
    uint32_t expiryTwo = (timeNow + tc2->timeQueued_) + tc2->duration_;
    
    if (expiryOne < expiryTwo)
    {
        retVal = -1;
    }
    else if (expiryOne > expiryTwo)
    {
        retVal = 1;
    }
    else // (expiryOne == expiryTwo)
    {
        retVal = 0;
    }
    
    return retVal;
}


























