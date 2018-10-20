#ifndef __FUNCTION_CHAIN_H__
#define __FUNCTION_CHAIN_H__


#include "Function.h"
#include "TimedEventHandler.h"


template <uint8_t CHAIN_LEN>
class FunctionChainAsync
{
private:

    struct FnData
    {
        function<void()> fn;
        uint32_t delayBeforeStartMs;
    };
    
public:
    FunctionChainAsync()
    {
        Reset();
    }

    void Reset()
    {
        Stop();

        for (auto &fnData : fnDataList_)
        {
            fnData.fn                 = [](){};
            fnData.delayBeforeStartMs = 0;
        }

        fnDataListAddNextIdx_ = 0;
        fnDataListRunNextIdx_ = 0;
        
        fnDataOnComplete_.fn                 = [](){};
        fnDataOnComplete_.delayBeforeStartMs = 0;
    }

    uint8_t Append(function<void()> fn, uint32_t delayBeforeStartMs = 0)
    {
        uint8_t retVal = 0;
        
        if (fnDataListAddNextIdx_ < CHAIN_LEN)
        {
            retVal = 1;
            
            fnDataList_[fnDataListAddNextIdx_] = { fn, delayBeforeStartMs };

            ++fnDataListAddNextIdx_;
        }
        
        return retVal;
    }
    
    void SetCallbackOnComplete(function<void()> cbFnOnComplete, uint32_t delayBeforeStartMs = 0)
    {
        fnDataOnComplete_ = { cbFnOnComplete, delayBeforeStartMs };
    }

    void Start()
    {
        Stop();

        if (fnDataListAddNextIdx_ != 0)
        {
            ted_.SetCallback([this](){
                OnTimeout();
            });
            
            ted_.RegisterForTimedEvent(fnDataList_[0].delayBeforeStartMs);
        }
        else
        {
            ScheduleCallbackOnComplete();
        }
    }

    void Stop()
    {
        ted_.DeRegisterForTimedEvent();
    }

private:

    void ScheduleCallbackOnComplete()
    {
        ted_.SetCallback(fnDataOnComplete_.fn);
        ted_.RegisterForTimedEvent(fnDataOnComplete_.delayBeforeStartMs);
    }

    void OnTimeout()
    {
        fnDataList_[fnDataListRunNextIdx_].fn();
        ++fnDataListRunNextIdx_;
        
        if (fnDataListRunNextIdx_ < fnDataListAddNextIdx_)
        {
            ted_.RegisterForTimedEvent(fnDataList_[fnDataListRunNextIdx_].delayBeforeStartMs);
        }
        else
        {
            ScheduleCallbackOnComplete();
        }
    }

    TimedEventHandlerDelegate ted_;

    FnData  fnDataList_[CHAIN_LEN];
    uint8_t fnDataListAddNextIdx_ = 0;
    uint8_t fnDataListRunNextIdx_ = 0;
    
    FnData fnDataOnComplete_;
};










#endif  // __FUNCTION_CHAIN_H__




