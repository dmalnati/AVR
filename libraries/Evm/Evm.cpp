#include <Arduino.h>

#include "Evm.h"


//////////////////////////////////////////////////////////////////////
//
// Idle Events
//
//////////////////////////////////////////////////////////////////////

void Evm::RegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh)
{
    idleTimeEventHandlerList_.PushBack(iteh);
}

void Evm::DeRegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh)
{
    idleTimeEventHandlerList_.Remove(iteh);
}

void Evm::ServiceIdleTimeEventHandlers()
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
    for (uint8_t i = 0; i < idleTimeEventHandlerList_.Size(); ++i)
    {
        idleTimeEventHandlerList_[i]->OnIdleTimeEvent();
    }
}



//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////


void Evm::RegisterTimedEventHandler(TimedEventHandler *teh, uint32_t duration)
{
    // Note what time it is when timer requested
    uint32_t timeNow = millis();
    
    // Keep track of some useful state
    teh->timeQueued_ = timeNow;
    teh->duration_   = duration;

    // Queue it
    timedEventHandlerList_.PushSorted(teh, CmpTimedEventHandler);
}

void Evm::DeRegisterTimedEventHandler(TimedEventHandler *teh)
{
    timedEventHandlerList_.Remove(teh);
}

void Evm::ServiceTimedEventHandlers()
{
    const uint8_t MAX_EVENTS_HANDLED = 4;
    
    if (timedEventHandlerList_.Size())
    {
        uint8_t            remainingEvents = MAX_EVENTS_HANDLED;
        bool               keepGoing       = true;
        TimedEventHandler *teh             = NULL;
        
        do {
            teh = timedEventHandlerList_.PeekFront();
            
            //uint32_t timeNow = micros();
            uint32_t timeNow = millis();
            
            // Check if the time since accepting event is gte than
            // the duration the event was supposed to wait for.
            // Handles wraparound this way.
            if ((timeNow - teh->timeQueued_) >= teh->duration_)
            {
                // drop this element from the list
                timedEventHandlerList_.PopFront();
                
                // invoke the IdleTimeEventHandler
                teh->OnTimedEvent();
                
                // re-schedule if it is an interval timer
                if (teh->isInterval_)
                {
                    RegisterTimedEventHandler(teh, teh->duration_);
                }
                
                // only keep going if remaining quota of events remains
                --remainingEvents;
                keepGoing = remainingEvents;
            }
            else
            {
                keepGoing = false;
            }
        } while (keepGoing && timedEventHandlerList_.Size());
    }
}

void Evm::MainLoop()
{
    while (1)
    {
        ServiceIdleTimeEventHandlers();
        ServiceTimedEventHandlers();
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
Evm::CmpTimedEventHandler(TimedEventHandler *teh1, TimedEventHandler *teh2)
{
    int8_t retVal;
    
    uint32_t timeNow = millis();
    
    uint32_t expiryOne = (timeNow + teh1->timeQueued_) + teh1->duration_;
    uint32_t expiryTwo = (timeNow + teh2->timeQueued_) + teh2->duration_;
    
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


























