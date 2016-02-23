#ifndef __TEST_NO_DYNAMIC_MEMORY_H__
#define __TEST_NO_DYNAMIC_MEMORY_H__



#include <stdint.h>


 
template <uint8_t CAPACITY>
class Container
{
public:
 
private:
    uint8_t table_[CAPACITY];
 
    uint8_t front_;
    uint8_t back_;
    uint8_t size_;
};
 
 
//////////////////////////////
 
 
 
template <uint8_t COUNT>
class FaderHelper
{
public:
    static const uint8_t COUNT_IDLE_TIME_EVENT_HANDLER = 0;
    static const uint8_t COUNT_TIMED_EVENT_HANDLER     = 1;
    static const uint8_t COUNT_INTERRUPT_EVENT_HANDLER = 0;
 
private:
    uint8_t table_[COUNT];
    // pretend I'm a timed event
};
 
 
template <uint8_t LED_COUNT>
class Fader
{
public:
    static const uint8_t COUNT_IDLE_TIME_EVENT_HANDLER = LED_COUNT * 2;
    static const uint8_t COUNT_TIMED_EVENT_HANDLER     = FaderHelper<LED_COUNT>::COUNT_TIMED_EVENT_HANDLER;
    static const uint8_t COUNT_INTERRUPT_EVENT_HANDLER = 0;
 
 
 
private:
    uint8_t member1_;
 
    Container<LED_COUNT> activeList_;
    Container<LED_COUNT> pendingList_;
    FaderHelper<LED_COUNT> helper_;
};
 
 
 
 
 
//////////////////////////////
 
 
template <typename EvmT>
class UserOfEvm
{
public:
    UserOfEvm(EvmT &evm)
    {
        evm.DoNothing();
    }
 
 
private:
};
 
 


template <typename SysT>
class IdleTimeEventHandler
{
public:
    IdleTimeEventHandler(SysT &sys);

private:
    SysT &sys_;
};



 
template <
    uint8_t COUNT_IDLE_TIME_EVENT_HANDLER,
    uint8_t COUNT_TIMED_EVENT_HANDLER,
    uint8_t COUNT_INTERRUPT_EVENT_HANDLER
> 
class Evm
{
    template <typename SysT>
    friend class IdleTimeEventHandler;
    
public:
    void MainLoop() { while (1) { } }
    void DoNothing() { }
 
private:
    void DoNothingPrivate() { }

    Container<COUNT_IDLE_TIME_EVENT_HANDLER> idleTimeEventHandlerList_;
    Container<COUNT_TIMED_EVENT_HANDLER>     timedEventHandlerList_;
    Container<COUNT_INTERRUPT_EVENT_HANDLER> interruptEventHandlerList_;
};
 


#include "TestNoDynamicMemory.hpp"






 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

#endif  // __TEST_NO_DYNAMIC_MEMORY_H__