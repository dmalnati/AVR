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


void Evm::RegisterTimedEventHandler(TimedEventHandler *teh, uint32_t timeout)
{
    // Note what time it is when timer requested
    uint32_t timeNow = millis();
    
    // Keep track of some useful state
    teh->timeQueued_ = timeNow;
    teh->timeout_   = timeout;

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
            // the timeout the event was supposed to wait for.
            // Handles wraparound this way.
            if ((timeNow - teh->timeQueued_) >= teh->timeout_)
            {
                // drop this element from the list
                timedEventHandlerList_.PopFront();
                
                // invoke the IdleTimeEventHandler
                teh->OnTimedEvent();
                
                // re-schedule if it is an interval timer
                if (teh->isInterval_)
                {
                    RegisterTimedEventHandler(teh, teh->timeout_);
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





//////////////////////////////////////////////////////////////////////
//
// Interrupt Events
//
//////////////////////////////////////////////////////////////////////


//
// MUST NOT ENABLE INTERRUPTS IN THIS FUNCTION
//
// This function is only called from an ISR.
//
// That ISR is:
// - already in a state of interrupts being suppressed
// - going to break if interrupts are re-enabled here and another ISR fires
//   before the whole operation completes.
//
void Evm::RegisterInterruptEventHandler(InterruptEventHandler *ieh)
{
    uint8_t idxLogical = 0;
    
    // Prevent this event from being added more than once
    if (!interruptEventHandlerList_.FindIdxFirst(ieh, idxLogical))
    {
        interruptEventHandlerList_.PushBack(ieh);
    }
}

//
// This function is only called from the "main thread."
//
// As a result, access to ISR-changeable structures must be protected,
// as well as any logic which relies on those structures remaining static.
//
void Evm::DeRegisterInterruptEventHandler(InterruptEventHandler *ieh)
{
    noInterrupts();
    
    interruptEventHandlerList_.Remove(ieh);
    
    interrupts();
}

//
// This function is only called from the "main thread."
//
// As a result, access to ISR-changeable structures must be protected,
// as well as any logic which relies on those structures remaining static.
//
void Evm::ServiceInterruptEventHandlers()
{
    const uint8_t MAX_EVENTS_HANDLED = 4;
    
    uint8_t remainingEvents = MAX_EVENTS_HANDLED;
    
    // Suppress interrupts during critical sections of code
    noInterrupts();
    while (interruptEventHandlerList_.Size() && remainingEvents)
    {
        InterruptEventHandler *ieh = interruptEventHandlerList_.PopFront();
        interrupts();
        
        // No need to disable interrupts here, ISR-invoked code only modifies
        // the interruptEventHandlerList_.
        //
        // Everything else behaves like normal.
        ieh->OnInterruptEvent();
 
        // Keep track of remaining events willing to handle
        --remainingEvents;
        
        // Suppress interrupts, about to loop around and look at list again
        noInterrupts();
    }
    
    // Re-Enable interrupts
    interrupts();
}








//////////////////////////////////////////////////////////////////////
//
// Main Loop
//
//////////////////////////////////////////////////////////////////////


void Evm::MainLoop()
{
    while (1)
    {
        ServiceIdleTimeEventHandlers();
        ServiceTimedEventHandlers();
        ServiceInterruptEventHandlers();
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
    
    uint32_t expiryOne = (timeNow + teh1->timeQueued_) + teh1->timeout_;
    uint32_t expiryTwo = (timeNow + teh2->timeQueued_) + teh2->timeout_;
    
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


























