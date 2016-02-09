#ifndef __EVM_H__
#define __EVM_H__


#include <stdint.h>


#include "EvmEventHandler.h"
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
    {
        // nothing to do
    }
    
    
    // Idle Events
    void RegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh);
    void DeRegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh);
    
    void ServiceIdleTimeEventHandlers();
    
    
    // Timed Events
    void RegisterTimedEventHandler(TimedEventHandler *teh, uint32_t duration );
    void DeRegisterTimedEventHandler(TimedEventHandler *teh);
    
    void ServiceTimedEventHandlers();

    
    // Members
    MyStaqueue<IdleTimeEventHandler *> idleTimeEventHandlerList_;
    MyStaqueue<TimedEventHandler *>    timedEventHandlerList_;
};


#endif  // __EVM_H__







