#ifndef __EVM_H__
#define __EVM_H__


#include <stdint.h>


#include "PAL.h"
#include "IdleTimeEventHandler.h"
#include "TimedEventHandler.h"
#include "InterruptEventHandler.h"
#include "MyStaqueue.h"


class Evm
{
    friend class IdleTimeEventHandler;
    friend class TimedEventHandler;
    
public:
    ~Evm() {}
    
    void MainLoop();
    
    static Evm &GetInstance(uint8_t maxEventCapacity = MAX_EVENT_CAPACITY);
    static int8_t CmpTimedEventHandler(TimedEventHandler *teh1,
                                       TimedEventHandler *teh2);
    
private:
    static const uint8_t MAX_EVENT_CAPACITY = 8;

    
    // Can't construct directly
    Evm(uint8_t maxEventCapacity)
    : idleTimeEventHandlerList_(maxEventCapacity)
    , timedEventHandlerList_(maxEventCapacity)
    , interruptEventHandlerList_(maxEventCapacity)
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
    MyStaqueue<IdleTimeEventHandler *>  idleTimeEventHandlerList_;
    MyStaqueue<TimedEventHandler *>     timedEventHandlerList_;
    
    // This is a data structure which needs to be carefully managed.
    //
    // It can be accessed both from ISR-driven code as well as typical
    // "main thread" code.
    //
    // As a result, any code which accesses this structure must be
    // written with a full appreciation of which code is driving
    // its execution and prevent corruption of its data
    // as well as any logic making use of its data.
    MyStaqueue<InterruptEventHandler *> interruptEventHandlerList_;
};


#endif  // __EVM_H__







