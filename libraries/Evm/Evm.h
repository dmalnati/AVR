#ifndef __EVM_H__
#define __EVM_H__


#include <stdint.h>

#include <Arduino.h>

#include "EvmCallback.h"
#include "MyStaqueue.h"


class Evm
{
    friend class IdleCallback;
    friend class TimedCallback;
    
public:
    ~Evm() {}
    
    void MainLoop();
    
    static Evm &
    GetInstance(uint8_t maxEventCapacity = DEFAULT_MAX_EVENT_CAPACITY)
    {
        static Evm *evm = NULL;
        
        if (!evm)
        {
            evm = new Evm(maxEventCapacity);
        }
        
        return *evm;
    }

    static int8_t CmpTimedCallback(TimedCallback *tc1, TimedCallback *tc2)
    {
        int8_t retVal;
        
        uint32_t timeNow = millis();
        
        uint32_t expiryOne = (timeNow + tc1->timeQueued_) + tc1->duration_;
        uint32_t expiryTwo = (timeNow + tc2->timeQueued_) + tc2->duration_;
        
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
    
private:
    static const uint8_t DEFAULT_MAX_EVENT_CAPACITY = 8;

    // Can't construct directly
    Evm(uint8_t maxEventCapacity)
    : timedEventList_(maxEventCapacity)
    , idleEventList_(maxEventCapacity)
    {
        // nothing to do
    }
    
    void SetTimeout(uint32_t duration, TimedCallback *cbo);
    void SetTimeoutMs(uint32_t duration, TimedCallback *cbo);
    void SetIdleCallback(Callback *cbo);

    void HandleTimers();
    void HandleIdleFunctions();
    
    MyStaqueue<TimedCallback *> timedEventList_;
    MyStaqueue<Callback *>      idleEventList_;
};








#endif  // __EVM_H__







