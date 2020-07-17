#ifndef __DURATION_AUDITOR_H__
#define __DURATION_AUDITOR_H__


#include "PAL.h"
#include "StrFormat.h"


template <uint8_t EVENT_COUNT, uint32_t(*TIME_FUNCTION)()>
class DurationAuditorBase
{
private:
    struct Event
    {
        const char *eventName = "";
        uint32_t    eventTime = 0;
    };

public:
    DurationAuditorBase()
    : fnGetTime_(TIME_FUNCTION)
    {
        Reset();
    }

    void Reset()
    {
        eventListIdx_ = 0;

        for (Event &event : eventList_)
        {
            event = Event{};
        }
    }

    void Audit(const char *eventName)
    {
        Audit(eventName, fnGetTime_());
    }

    void Audit(const char *eventName, uint32_t eventTime)
    {
        if (eventListIdx_ < EVENT_COUNT)
        {
            eventList_[eventListIdx_] = Event{eventName, eventTime};

            ++eventListIdx_;
        }
    }

    void Report()
    {
        uint8_t eventCount = eventListIdx_;

        if (eventCount > 1)
        {
            Report(0, eventCount - 1);

            if (eventCount > 2)
            {
                for (uint8_t i = 1; i < eventCount; ++i)
                {
                    Report(i - 1, i);
                }
            }
        }
    }

    void Report(uint8_t eventNum1, uint8_t eventNum2)
    {
        if (eventNum1 < eventListIdx_ && eventNum2 < eventListIdx_)
        {
            Event &first = eventList_[eventNum1];
            Event &last  = eventList_[eventNum2];

            AuditPair(first, last);
        }
    }

private:

    void AuditPair(Event &prior, Event &curr)
    {
        uint32_t timeDiff = curr.eventTime - prior.eventTime;

        char buf[14];
        StrFormat::U32ToStrCommas(buf, timeDiff);

        // Support hundreds of millisecons
        // for ms mode, this is 3 digits
        // for us mode, this is 7 digits
        const char *formatStr = "%3s ms: %s -> %s\n";
        if (fnGetTime_ == PAL.Micros)
        {
            formatStr = "%7s us: %s -> %s\n";
        }

        printf(formatStr,
               buf,
               prior.eventName,
               curr.eventName);
    }

    using TimeFn = uint32_t(*)();
    TimeFn fnGetTime_;
    
    Event eventList_[EVENT_COUNT];

    uint8_t eventListIdx_;
};


template <uint8_t EVENT_COUNT>
using DurationAuditorMillis = DurationAuditorBase<EVENT_COUNT, PlatformAbstractionLayer::Millis>;

template <uint8_t EVENT_COUNT>
using DurationAuditor = DurationAuditorBase<EVENT_COUNT, PlatformAbstractionLayer::Micros>;



#endif  // __DURATION_AUDITOR_H__


