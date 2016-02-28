#ifndef __INTERRUPT_EVENT_HANDLER_H__
#define __INTERRUPT_EVENT_HANDLER_H__


#include <util/atomic.h>

#include <stdint.h>
#include <stdlib.h>

#include "PCIntEventHandler.h"


// Forward declaration
template <uint8_t COUNT_IDLE_TIME_EVENT_HANDLER,
      uint8_t COUNT_TIMED_EVENT_HANDLER,
      uint8_t COUNT_INTERRUPT_EVENT_HANDLER>
class EvmActual;


#define LEVEL_UNDEFINED           0
#define LEVEL_FALLING             1
#define LEVEL_RISING              2
#define LEVEL_RISING_AND_FALLING  3


/*
 * Main Principle of Interrupt Synchronization Architecture:
 * - ISR-invoked code can only add to Evm InterruptEventHandler list
 *   - and never enables interrupts while running
 *
 * - Evm-invoked code can read/write the InterruptEventHandler list
 *   - but does so only in protected code which prevents ISRs from firing
 *     - and takes care to not rely on the structure being unchanged outside
 *       of protected areas.
 *
 * - This means most complexity still lies in main thread code, ISRs simply
 *   queue data for Evm to deal with later.
 *
 * - Additionally this means there can only be 1 of 2 types of code running
 *   while accessing data which can be reached from both ISR and Evm.
 *
 * - Lastly this makes ISR-invoked code return very quickly, maximizing
 *   opportunity for other ISRs to be serviced, since they are blocked out
 *   while other ISRs are running.
 *
 *
 */

 
//////////////////////////////////////////////////////////////////////
//
// Interrupt Events
//
//////////////////////////////////////////////////////////////////////

class InterruptEventHandler
: private PCIntEventHandler
{
    template <uint8_t, uint8_t, uint8_t>
    friend class EvmActual;
    
public:
    InterruptEventHandler()
    : mode_(LEVEL_UNDEFINED)
    , logicLevel_(0)
    {
        // nothing to do
    }
    
    ~InterruptEventHandler()
    {
        DeRegisterForInterruptEvent();
    }

    uint8_t GetMode()       const { return mode_; }
    uint8_t GetLogicLevel() const { return logicLevel_; }
    
    //////////////////////////////////////////////////////////////////////
    //
    // These functions are only called from "Main Thread" code.
    //
    // They do not require protection from reentrant effects since
    // that is handled downstream.
    //
    //////////////////////////////////////////////////////////////////////

    uint8_t RegisterForInterruptEvent(uint8_t pin,
                                      uint8_t mode = LEVEL_RISING);
    uint8_t DeRegisterForInterruptEvent();
    
    virtual void OnInterruptEvent(uint8_t logicLevel) = 0;
    
private:
    void OnInterruptEventPrivate()
    {
        OnInterruptEvent(GetLogicLevel());
    }
    
    PCIntEventHandler::MODE ConvertToPCIntEventHandlerMode(uint8_t mode) const
    {
        PCIntEventHandler::MODE retVal;
        
        if (mode == LEVEL_RISING)
        {
            retVal = PCIntEventHandler::MODE::MODE_RISING;
        }
        else if (mode == LEVEL_FALLING)
        {
            retVal = PCIntEventHandler::MODE::MODE_FALLING;
        }
        else if (mode == LEVEL_RISING_AND_FALLING)
        {
            retVal = PCIntEventHandler::MODE::MODE_RISING_AND_FALLING;
        }
        else
        {
            retVal = PCIntEventHandler::MODE::MODE_UNDEFINED;
        }
        
        return retVal;
    }
    

    // Implement the PCIntEventHandler interface
    
    // This is only called from ISR code.  Must not enable interrupts here or
    // anywhere it leads to.
    virtual void OnPCIntEvent(uint8_t logicLevel);
    
    
    uint8_t mode_;
    uint8_t logicLevel_;
};




//////////////////////////////////////////////////////////////////////
//
// Object Wrappers
//
//////////////////////////////////////////////////////////////////////

template <typename T>
class InterruptEventHandlerDelegate
: public InterruptEventHandler
{
    typedef void (T::*MemberCallbackFn)(uint8_t logicLevel);
    
public:
    void SetCallback(T *obj, MemberCallbackFn func)
    {
        obj_  = obj;
        func_ = func;
    }

private:
    virtual void OnInterruptEvent(uint8_t logicLevel)
    {
        if (obj_ && func_) { ((*obj_).*func_)(logicLevel); }
    }

    T                *obj_;
    MemberCallbackFn  func_;
};




#endif  // __INTERRUPT_EVENT_HANDLER_H__





















