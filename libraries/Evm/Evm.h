#ifndef __EVM_H__
#define __EVM_H__


#include <stdint.h>


#include "EvmCallback.h"
#include "MyStaqueue.h"


class Evm
{
    friend class IdleCallback;
    friend class TimedCallback;
    
public:
    ~Evm() {}
    
    void MainLoop();
    
    static Evm &GetInstance(uint8_t maxEventCapacity = MAX_EVENT_CAPACITY);
    static int8_t CmpTimedCallback(TimedCallback *tc1, TimedCallback *tc2);
    
private:
    static const uint8_t MAX_EVENT_CAPACITY = 8;

    
    // Can't construct directly
    Evm(uint8_t maxEventCapacity)
    : timedEventList_(maxEventCapacity)
    , idleEventList_(maxEventCapacity)
    {
        // nothing to do
    }
    
    
    // Idle Events
    void SetIdleCallback(Callback *cbo);
    void CancelIdleCallback(Callback *cbo);
    void HandleIdleFunctions();
    
    
    // Timed Events
    void SetTimeout(uint32_t duration, TimedCallback *cbo);
    void SetTimeoutMs(uint32_t duration, TimedCallback *cbo);
    void CancelTimeout(TimedCallback *cbo);
    void HandleTimers();

    
    // Members
    MyStaqueue<TimedCallback *> timedEventList_;
    MyStaqueue<Callback *>      idleEventList_;
};


#endif  // __EVM_H__







