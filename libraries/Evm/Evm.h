#ifndef __EVM_H__
#define __EVM_H__

#include <stdint.h>
 
 
 
// Include just because a lot of stuff uses it
#include "PAL.h"

// Friends
#include "IdleTimeEventHandler.h"
#include "TimedEventHandler.h"
#include "InterruptEventHandler.h"
 
 
// Forward declaration
template <uint8_t COUNT_IDLE_TIME_EVENT_HANDLER,
          uint8_t COUNT_TIMED_EVENT_HANDLER,
          uint8_t COUNT_INTERRUPT_EVENT_HANDLER>
class EvmActual;
 
 
// Interface definition and some core functionality
class Evm
{
    // Declare EvmActual as a friend so it can construct
    template <uint8_t COUNT_IDLE_TIME_EVENT_HANDLER,
          uint8_t COUNT_TIMED_EVENT_HANDLER,
          uint8_t COUNT_INTERRUPT_EVENT_HANDLER>
    friend class EvmActual;
    friend class IdleTimeEventHandler;
    friend class TimedEventHandler;
    friend class InterruptEventHandler;

    //////////////////////////////////////////////////////////////////////
    //
    // Evm usual Interfaces, both public and private
    //
    //////////////////////////////////////////////////////////////////////
public:
    virtual
    void MainLoop();
    
private:

    // Idle Events
    virtual
    uint8_t RegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh)   = 0;
    virtual
    uint8_t DeRegisterIdleTimeEventHandler(IdleTimeEventHandler *iteh) = 0;
    
    // Timed Events
    virtual
    uint8_t RegisterTimedEventHandler(TimedEventHandler *teh,
                                      uint32_t timeout)         = 0;
    virtual
    uint8_t DeRegisterTimedEventHandler(TimedEventHandler *teh) = 0;
    
    // Interrupt Events
    virtual
    uint8_t RegisterInterruptEventHandler(InterruptEventHandler *ieh)   = 0;
    virtual
    uint8_t DeRegisterInterruptEventHandler(InterruptEventHandler *ieh) = 0;
    

    
    
//////////////////////////////////////////////////////////////////////
//
// Evm Global Interfaces
//
//////////////////////////////////////////////////////////////////////

public:
    // Static classes to get access to the single constructed template instance
    static Evm &GetInstance()
    {
        return *evm_;
    }
 
    // This function must be called before any call to GetInstance.
    // Typically this would be first member initializer to a local Evm reference
    template <uint8_t COUNT_IDLE_TIME_EVENT_HANDLER,
              uint8_t COUNT_TIMED_EVENT_HANDLER,
              uint8_t COUNT_INTERRUPT_EVENT_HANDLER>
    static Evm &CreateInstance()
    {
        // Create statically allocated template instance
        static EvmActual<COUNT_IDLE_TIME_EVENT_HANDLER,
                         COUNT_TIMED_EVENT_HANDLER,
                         COUNT_INTERRUPT_EVENT_HANDLER> evm;
 
        // Make available a pointer to this instance
        //
        // If this function is called more than once, it's a huge problem, but
        // it can be limited by at least keeping only a single Evm instance
        // available to the outside world.
        if (!evm_)
        {
            evm_ = &evm;
        }
 
        return *evm_;
    }
 
 
private:
    // Can't instantiate directly, singleton
    Evm() { }
 
    // The single class-wide EvmActual
    static Evm *evm_;
};
 

// Bring in actual template implementation
#include "EvmActual.h"





#endif  // __EVM_H__