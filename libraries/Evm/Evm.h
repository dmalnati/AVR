#ifndef __EVM_H__
#define __EVM_H__


#include <stdint.h>


#include "PAL.h"
#include "IdleTimeEventHandler.h"
#include "TimedEventHandler.h"
#include "InterruptEventHandler.h"
#include "Container.h"



template <
    uint8_t COUNT_IDLE_TIME_EVENT_HANDLER,
    uint8_t COUNT_TIMED_EVENT_HANDLER,
    uint8_t COUNT_INTERRUPT_EVENT_HANDLER
>
class Evm
{
    typedef
    Evm<COUNT_IDLE_TIME_EVENT_HANDLER,
        COUNT_TIMED_EVENT_HANDLER,
        COUNT_INTERRUPT_EVENT_HANDLER
    > EvmT;
    
    
    friend class IdleTimeEventHandler<EvmT>;
    friend class TimedEventHandler<EvmT>;
    friend class InterruptEventHandler<EvmT>;
    
    
public:
    Evm()
    : idleTimeEventHandlerList_()
    , timedEventHandlerList_()
    , interruptEventHandlerList_()
    {
        // nothing to do
    }

    void MainLoop();
    
    template <typename EvmT>
    class CmpTimedEventHandler
    {
    public:
        uint8_t operator()(TimedEventHandler<EvmT> *teh1,
                           TimedEventHandler<EvmT> *teh2)
        {
            int8_t retVal;
            
            uint32_t timeNow = PAL.Millis();
            
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

    // Idle Events
    uint8_t RegisterIdleTimeEventHandler(IdleTimeEventHandler<EvmT> *iteh);
    uint8_t DeRegisterIdleTimeEventHandler(IdleTimeEventHandler<EvmT> *iteh);
    
    void ServiceIdleTimeEventHandlers();


    // Timed Events
    uint8_t RegisterTimedEventHandler(TimedEventHandler<EvmT> *teh, uint32_t timeout);
    uint8_t DeRegisterTimedEventHandler(TimedEventHandler<EvmT> *teh);
    
    void ServiceTimedEventHandlers();
    

    // Interrupt Events
    uint8_t RegisterInterruptEventHandler(InterruptEventHandler<EvmT> *ieh);
    uint8_t DeRegisterInterruptEventHandler(InterruptEventHandler<EvmT> *ieh);
    
    void ServiceInterruptEventHandlers();

    
    
    // Members
    Queue<IdleTimeEventHandler<EvmT> *,
          COUNT_IDLE_TIME_EVENT_HANDLER>  idleTimeEventHandlerList_;
         
    SortedQueue<TimedEventHandler<EvmT> *,
                COUNT_TIMED_EVENT_HANDLER,
                CmpTimedEventHandler<EvmT>>  timedEventHandlerList_;


    // This is a data structure which needs to be carefully managed.
    //
    // It can be accessed both from ISR-driven code as well as typical
    // "main thread" code.
    //
    // As a result, any code which accesses this structure must be
    // written with a full appreciation of which code is driving
    // its execution and prevent corruption of its data
    // as well as any logic making use of its data.
    Queue<InterruptEventHandler<EvmT> *,
          COUNT_INTERRUPT_EVENT_HANDLER>  interruptEventHandlerList_;
};





#include "Evm.hpp"




#endif  // __EVM_H__







