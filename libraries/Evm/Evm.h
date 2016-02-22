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
    
    static Evm &GetInstance();
    
    void MainLoop();
    void EndMainLoop();
    
    // Functionality to allow nested MainLoops for the purpose of
    // holding a given stack frame such that statics or other
    // async state keeping by users is less necessary.
    //
    // Failure scenarios include:
    // - The first nested stack sets a timer for 10ms to go off
    // - Something else running holds the stack also, with a timeout
    //   for 100ms.
    // - The first 10ms timer goes off, breaking out of the second level
    //   and leading to unpredictable results at both levels.
    //
    // The calling code must know exactly what is going on in order for
    // this to not happen.
    //
    // The assertion code forces a correct statement about the stack
    // level in order to fail earlier and more predictably.
    void HoldStackDangerously(uint8_t stackLevelAssertion, uint32_t timeout);
    
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
    : //stackLevel_(0)
    //, abort_(0)
    //, 
    idleTimeEventHandlerList_(INITIAL_EVENT_CAPACITY)
    , timedEventHandlerList_(INITIAL_EVENT_CAPACITY)
    , interruptEventHandlerList_(INITIAL_EVENT_CAPACITY)
    {
        // nothing to do
    }
    
    
    //
    // Supporting functionality for HoldStackDangerously
    //
    #if 0
    void DecrementStack();
    
    class DecrementStackOnTimeout
    : public TimedEventHandler
    {
    public:
        virtual void OnTimedEvent()
        {
            Evm::GetInstance().DecrementStack();
        }
    };
    #endif

    
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
    //uint8_t stackLevel_;
    //uint8_t abort_;
    
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







