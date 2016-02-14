#ifndef __EVM_H__
#define __EVM_H__


#include <stdint.h>


#include "PAL.h"
#include "IdleTimeEventHandler.h"
#include "TimedEventHandler.h"
#include "InterruptEventHandler.h"
#include "Container.h"




class Evm
{
    friend class IdleTimeEventHandler;
    friend class TimedEventHandler;
    
public:
    ~Evm() {}
    
    void MainLoop();
    
    static Evm &GetInstance();
    
    class CmpTimedEventHandler
    {
    public:
        uint8_t operator()(TimedEventHandler *teh1, TimedEventHandler *teh2)
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
    };
    
private:
    static const uint8_t INITIAL_EVENT_CAPACITY = 4;

    
    // Can't construct directly
    Evm()
    : idleTimeEventHandlerList_(INITIAL_EVENT_CAPACITY)
    , timedEventHandlerList_(INITIAL_EVENT_CAPACITY)
    , interruptEventHandlerList_(INITIAL_EVENT_CAPACITY)
    {
        // nothing to do
    }
    
    
    // Idle Events
    void RegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh);
    void DeRegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh);
    
    void ServiceIdleTimeEventHandlers();
    
    
    // Timed Events
    void RegisterTimedEventHandler(TimedEventHandler *teh, uint32_t timeout);
    void DeRegisterTimedEventHandler(TimedEventHandler *teh);
    
    void ServiceTimedEventHandlers();
    
    
    // Interrupt Events
public: // needed so ISR static functions can actually (de)register events
    void RegisterInterruptEventHandler(InterruptEventHandler *ieh);
    void DeRegisterInterruptEventHandler(InterruptEventHandler *ieh);
private:    
    void ServiceInterruptEventHandlers();

    
    // Members
    Queue<IdleTimeEventHandler *>      idleTimeEventHandlerList_;
    SortedQueue<TimedEventHandler *,
                CmpTimedEventHandler>  timedEventHandlerList_;
    
    // This is a data structure which needs to be carefully managed.
    //
    // It can be accessed both from ISR-driven code as well as typical
    // "main thread" code.
    //
    // As a result, any code which accesses this structure must be
    // written with a full appreciation of which code is driving
    // its execution and prevent corruption of its data
    // as well as any logic making use of its data.
    Queue<InterruptEventHandler *> interruptEventHandlerList_;
};


#endif  // __EVM_H__







