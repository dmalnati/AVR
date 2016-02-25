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