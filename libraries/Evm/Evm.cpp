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

void Evm::HandleIdleFunctions()
{
    const uint8_t size = idleEventList_.Size();
    
    for (uint8_t i = 0; i < size; ++i)
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