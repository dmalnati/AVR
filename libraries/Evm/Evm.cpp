#include <avr/interrupt.h>

#include "Evm.h"


//////////////////////////////////////////////////////////////////////
//
// Idle Events
//
//////////////////////////////////////////////////////////////////////

void Evm::RegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh)
{
    idleTimeEventHandlerList_.Push(iteh);
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
    uint32_t timeNow = PAL.Millis();
    
    // Keep track of some useful state
    teh->timeQueued_ = timeNow;
    teh->timeout_   = timeout;

    // Queue it
    timedEventHandlerList_.Push(teh);
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
        TimedEventHandler *tehTmp          = NULL;
        
        do {
            timedEventHandlerList_.Peek(teh);
            
            uint32_t timeNow = PAL.Millis();
            
            // Check if the time since accepting event is gte than
            // the timeout the event was supposed to wait for.
            // Handles wraparound this way.
            if ((timeNow - teh->timeQueued_) >= teh->timeout_)
            {
                // drop this element from the list
                timedEventHandlerList_.Pop(tehTmp);
                
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
    // Prevent this event from being added more than once
    if (!interruptEventHandlerList_.HasElement(ieh))
    {
        interruptEventHandlerList_.Push(ieh);
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
    cli();
    
    interruptEventHandlerList_.Remove(ieh);
    
    sei();
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
    
    InterruptEventHandler *ieh = NULL;
    
    // Suppress interrupts during critical sections of code
    cli();
    while (interruptEventHandlerList_.Size() && remainingEvents)
    {
        interruptEventHandlerList_.Pop(ieh);
        sei();
        
        // No need to disable interrupts here, ISR-invoked code only modifies
        // the interruptEventHandlerList_.
        //
        // Everything else behaves like normal.
        ieh->OnInterruptEvent();
 
        // Keep track of remaining events willing to handle
        --remainingEvents;
        
        // Suppress interrupts, about to loop around and look at list again
        cli();
    }
    
    // Re-Enable interrupts
    sei();
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
Evm::GetInstance()
{
    static Evm *evm = NULL;
    
    if (!evm)
    {
        evm = new Evm();
    }
    
    return *evm;
}



























