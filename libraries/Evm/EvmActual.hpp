#include <avr/interrupt.h>

#include <util/atomic.h>



//////////////////////////////////////////////////////////////////////
//
// Idle Events
//
//////////////////////////////////////////////////////////////////////

template <uint8_t A, uint8_t B, uint8_t C>
uint8_t EvmActual<A,B,C>::
RegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh)
{
    return idleTimeEventHandlerList_.Push(iteh);
}

template <uint8_t A, uint8_t B, uint8_t C>
uint8_t EvmActual<A,B,C>::
DeRegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh)
{
    return idleTimeEventHandlerList_.Remove(iteh);
}

template <uint8_t A, uint8_t B, uint8_t C>
void EvmActual<A,B,C>::
ServiceIdleTimeEventHandlers()
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


template <uint8_t A, uint8_t B, uint8_t C>
uint8_t EvmActual<A,B,C>::
RegisterTimedEventHandler(TimedEventHandler *teh, uint32_t timeout)
{
    // Note what time it is when timer requested
    uint32_t timeNow = PAL.Millis();
    
    // Keep track of some useful state
    teh->timeQueued_ = timeNow;
    teh->timeout_    = timeout;

    // Queue it
    return timedEventHandlerList_.Push(teh);
}

template <uint8_t A, uint8_t B, uint8_t C>
uint8_t EvmActual<A,B,C>::
DeRegisterTimedEventHandler(TimedEventHandler *teh)
{
    return timedEventHandlerList_.Remove(teh);
}

template <uint8_t A, uint8_t B, uint8_t C>
void EvmActual<A,B,C>::
ServiceTimedEventHandlers()
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
template <uint8_t A, uint8_t B, uint8_t C>
uint8_t EvmActual<A,B,C>::
RegisterInterruptEventHandler(InterruptEventHandler *ieh)
{
    uint8_t retVal = 1;
    
    // Prevent this event from being added more than once
    if (!interruptEventHandlerList_.HasElement(ieh))
    {
        retVal = interruptEventHandlerList_.Push(ieh);
    }
    
    return retVal;
}

//
// This function is only called from the "main thread."
//
// As a result, access to ISR-changeable structures must be protected,
// as well as any logic which relies on those structures remaining static.
//
template <uint8_t A, uint8_t B, uint8_t C>
uint8_t EvmActual<A,B,C>::
DeRegisterInterruptEventHandler(InterruptEventHandler *ieh)
{
    uint8_t retVal;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        retVal = interruptEventHandlerList_.Remove(ieh);
    }
    
    return retVal;
}

//
// This function is only called from the "main thread."
//
// As a result, access to ISR-changeable structures must be protected,
// as well as any logic which relies on those structures remaining static.
//
template <uint8_t A, uint8_t B, uint8_t C>
void EvmActual<A,B,C>::
ServiceInterruptEventHandlers()
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
        ieh->OnInterruptEventPrivate();
 
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

template <uint8_t A, uint8_t B, uint8_t C>
void EvmActual<A,B,C>::
MainLoop()
{
    if (!mainLoopStackLevel_ && !mainLoopStackLevelTemporary_)
    {
        // One-way latch.  Not decremented later.
        // No support for re-starting the MainLoop later.
        ++mainLoopStackLevel_;
        
        MainLoopInternal();
    }
    else
    {
        // One of two possible problems:
        // - MainLoop was called while another MainLoop was still on the stack.
        //   - If that is desired, use HoldStackDangerously.
        // - or that the MainLoop was called while HoldStackDangerously
        //   was still on the stack.
        //   - That simply isn't supported.
        // 
        // Either way it's an error.
        
        PAL.SoftReset();
    }
 }

template <uint8_t A, uint8_t B, uint8_t C>
void EvmActual<A,B,C>::
MainLoopInternal()
{
    while (mainLoopKeepGoing_)
    {
        ServiceIdleTimeEventHandlers();
        ServiceTimedEventHandlers();
        ServiceInterruptEventHandlers();
    }
}

template <uint8_t A, uint8_t B, uint8_t C>
void EvmActual<A,B,C>::
EndMainLoop()
{
    mainLoopKeepGoing_ = 0;
}

template <uint8_t A, uint8_t B, uint8_t C>
void EvmActual<A,B,C>::
HoldStackDangerously(uint32_t timeout)
{
    if (!mainLoopStackLevelTemporary_)
    {
        ++mainLoopStackLevelTemporary_;
        
        if (timeout)
        {
            EndMainLoopOnTimeout emlot(timeout);
            
            MainLoopInternal();
        }
        else
        {
            MainLoopInternal();
        }
        
        mainLoopKeepGoing_ = 1;
        
        --mainLoopStackLevelTemporary_;
    }
    else
    {
        // Problem:
        // - HoldStackDangerously was already on the stack when called again.
        //   - Nested held stacks aren't supported.

        PAL.SoftReset();
    }
}


template <uint8_t A, uint8_t B, uint8_t C>
void EvmActual<A,B,C>::
MainLoopLowPower()
{
    while (1)
    {
        // If low power is enabled and there are no timed events queued, then
        // there never will be, therefore the DelaySleep will never fire, and
        // the device will consume regular power.
        // This shouldn't happen and isn't accounted for.
        if (lowPowerEnabled_ && timedEventHandlerList_.Size())
        {
            TimedEventHandler *teh = NULL;
            timedEventHandlerList_.Peek(teh);
            
            uint32_t timePassedSinceQueuing = PAL.Millis() - teh->timeQueued_;
            
            // check if event is in the future
            if (timePassedSinceQueuing < teh->timeout_)
            {
                uint32_t timeRemaining = teh->timeout_ - timePassedSinceQueuing;
                
                PAL.DelayLowPower(timeRemaining);
            }
        }
        
        ServiceTimedEventHandlers();
    }
}

template <uint8_t A, uint8_t B, uint8_t C>
void EvmActual<A,B,C>::
LowPowerEnable()
{
    lowPowerEnabled_ = 1;
}

template <uint8_t A, uint8_t B, uint8_t C>
void EvmActual<A,B,C>::
LowPowerDisable()
{
    lowPowerEnabled_ = 0;
}



















