#ifndef __EVM_ACTUAL_H__
#define __EVM_ACTUAL_H__


#include "Evm.h"
#include "Container.h"



template <uint8_t COUNT_IDLE_TIME_EVENT_HANDLER,
          uint8_t COUNT_TIMED_EVENT_HANDLER,
          uint8_t COUNT_INTERRUPT_EVENT_HANDLER>
class EvmActual
: public Evm
{
    friend class IdleTimeEventHandler;
    friend class TimedEventHandler;
    friend class InterruptEventHandler;
    template <typename T, uint8_t CAPACITY, typename CMP>
    friend class SortedQueue;
    
public:
    EvmActual()
    : mainLoopStackLevel_(0)
    , mainLoopStackLevelTemporary_(0)
    , mainLoopKeepGoing_(1)
    , lowPowerEnabled_(1)
    {
        // Store self
        Evm::evm_ = this;
    }

    
    //////////////////////////////////////////////////////////////////////
    //
    // EvmActual usual Interfaces, both public and private
    //
    //////////////////////////////////////////////////////////////////////
    virtual
    void MainLoop();
    
    virtual
    void HoldStackDangerously(uint32_t timeout = 0);
    
    virtual
    void EndMainLoop();
    
    virtual
    void MainLoopLowPower();
    
    virtual
    void LowPowerEnable();
    
    virtual
    void LowPowerDisable();
    
    virtual uint8_t GetIdleEventCount();
    virtual uint8_t GetTimedEventCount();
    virtual uint8_t GetInterruptEventCount();


private:

    void MainLoopInternal();
    
    class EndMainLoopOnTimeout
    : private TimedEventHandler
    {
    public:
        EndMainLoopOnTimeout(uint32_t timeout)
        {
            RegisterForTimedEvent(timeout);
        }
    
    private:
        virtual void OnTimedEvent()
        {
            Evm::GetInstance().EndMainLoop();
        }
    };
    
    

    // Idle Events
    virtual
    uint8_t RegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh);
    virtual
    uint8_t DeRegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh);
    
    void ServiceIdleTimeEventHandlers();


    // Timed Events
    virtual
    uint8_t RegisterTimedEventHandler(TimedEventHandler *teh,
                                      uint32_t timeout,
                                      uint32_t timeNow = PAL.Millis());
    virtual
    uint8_t DeRegisterTimedEventHandler(TimedEventHandler *teh);
    
    virtual
    uint8_t IsRegisteredTimedEventHandler(TimedEventHandler *teh);
    
    void ServiceTimedEventHandlers();
    

    // Interrupt Events
    virtual
    uint8_t RegisterInterruptEventHandler(InterruptEventHandler *ieh);
    virtual
    uint8_t DeRegisterInterruptEventHandler(InterruptEventHandler *ieh);
    
    void ServiceInterruptEventHandlers();

    
    //////////////////////////////////////////////////////////////////////
    //
    // EvmActual Private implementation details
    //
    //////////////////////////////////////////////////////////////////////
    
private:
    
    
    class CmpTimedEventHandler
    {
    public:
        uint8_t operator()(TimedEventHandler *teh1,
                           TimedEventHandler *teh2)
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

    
    // Main Loop members
    uint8_t mainLoopStackLevel_;
    uint8_t mainLoopStackLevelTemporary_;
    uint8_t mainLoopKeepGoing_;

    
    // Event Members
    Queue<IdleTimeEventHandler *,
          COUNT_IDLE_TIME_EVENT_HANDLER>  idleTimeEventHandlerList_;
         
    SortedQueue<TimedEventHandler *,
                1 +   // stack holder
                COUNT_TIMED_EVENT_HANDLER,
                CmpTimedEventHandler>  timedEventHandlerList_;


    // These are data structures which needs to be carefully managed.
    //
    // They can be accessed both from ISR-driven code as well as typical
    // "main thread" code.
    //
    // As a result, any code which accesses these structures must be
    // written with a full appreciation of which code is driving
    // its execution and prevent corruption of its data
    // as well as any logic making use of its data.
    Queue<InterruptEventHandler *,
          COUNT_INTERRUPT_EVENT_HANDLER>  interruptEventHandlerList_;
    
    uint8_t lowPowerEnabled_;
};





#include "EvmActual.hpp"




#endif  // __EVM_ACTUAL_H__







