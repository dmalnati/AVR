#ifndef __IDLE_TIME_HI_RES_TIMED_EVENT_HANDLER__
#define __IDLE_TIME_HI_RES_TIMED_EVENT_HANDLER__


#include "IdleTimeEventHandler.h"


//////////////////////////////////////////////////////////////////////
//
// Idle HiRes Timer Events
//
//////////////////////////////////////////////////////////////////////


// 60us between consecutive events seems to be the floor at 8MHz with no
// other activity going on.
// Whatever processing time for the handler only adds to that.


class IdleTimeHiResTimedEventHandler
: private IdleTimeEventHandler
{
public:
    IdleTimeHiResTimedEventHandler() : isInterval_(0) { }
    virtual ~IdleTimeHiResTimedEventHandler() { DeRegisterForIdleTimeHiResTimedEvent(); }
    
    uint8_t RegisterForIdleTimeHiResTimedEvent(uint32_t durationUs)
    {
        timeStart_  = PAL.Micros();
        durationUs_ = durationUs;
        isInterval_ = 0;
        
        return RegisterForIdleTimeEvent();
    }
    
    uint8_t RegisterForIdleTimeHiResTimedEventInterval(uint32_t durationUs)
    {
        uint8_t retVal = RegisterForIdleTimeHiResTimedEvent(durationUs);
        
        isInterval_ = 1;
        
        return retVal;
    }
    
    uint8_t DeRegisterForIdleTimeHiResTimedEvent()
    {
        isInterval_ = 0;
        
        return DeRegisterForIdleTimeEvent();
    }
    
    virtual void OnIdleTimeHiResTimedEvent() = 0;
    
private:
    virtual void OnIdleTimeEvent()
    {
        uint32_t timeNow = PAL.Micros();
        
        if (timeNow - timeStart_ >= durationUs_)
        {
            OnIdleTimeHiResTimedEvent();
            
            if (isInterval_)
            {
                timeStart_ = timeNow;
            }
            else
            {
                DeRegisterForIdleTimeHiResTimedEvent();
            }
        }
    }
    
    uint32_t timeStart_;
    uint32_t durationUs_;
    uint8_t  isInterval_;
};


//////////////////////////////////////////////////////////////////////
//
// Helpers
//
//////////////////////////////////////////////////////////////////////

class IdleTimeHiResTimedEventHandlerDelegate
: public IdleTimeHiResTimedEventHandler
{
public:
    IdleTimeHiResTimedEventHandlerDelegate()
    {
        // Nothing to do
    }
    
    IdleTimeHiResTimedEventHandlerDelegate(function<void()> cbFn)
    {
        SetCallback(cbFn);
    }

    void SetCallback(function<void()> cbFn)
    {
        cbFn_ = cbFn;
    }

private:
    virtual void OnIdleTimeHiResTimedEvent()
    {
        cbFn_();
    }

    function<void()> cbFn_;
};



#endif  // __IDLE_TIME_HI_RES_TIMED_EVENT_HANDLER__