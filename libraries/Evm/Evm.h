#ifndef __EVM_H__
#define __EVM_H__

#include <stdint.h>
 
 
 
// Include just because a lot of stuff uses it
#include "PAL.h"

// Friends
#include "IdleTimeEventHandler.h"
#include "TimedEventHandler.h"
#include "InterruptEventHandler.h"
#include "IdleTimeHiResTimedEventHandler.h"
 
 
// Forward declaration
template <uint8_t A, uint8_t B, uint8_t C>
class EvmActual;
 
 
// Interface definition and some core functionality
class Evm
{
    // Declare EvmActual as a friend so it can construct
    template <uint8_t A, uint8_t B, uint8_t C>
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
    virtual
    void HoldStackDangerously(uint32_t timeout = 0);
    
    virtual
    void EndMainLoop();
    
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
    
    template <uint8_t A, uint8_t B, uint8_t C>
    using Instance = EvmActual<A,B,C>;
 
 
protected:
    // Can't instantiate directly, singleton
    Evm() { }
 
    // The single class-wide EvmActual
    static Evm *evm_;
};
 

// Bring in actual template implementation
#include "EvmActual.h"





#endif  // __EVM_H__