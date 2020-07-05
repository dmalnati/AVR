#ifndef __DURATION_AUDITOR_H__
#define __DURATION_AUDITOR_H__


#include "PAL.h"


template <uint8_t EVENT_COUNT>
class DurationAuditor
{
private:
    struct Event
    {
        const char *eventName = "";
        uint32_t    eventTime = 0;
    };

public:
    DurationAuditor()
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
        if (eventListIdx_ < EVENT_COUNT)
        {
            eventList_[eventListIdx_] = Event{eventName, PAL.Millis()};

            ++eventListIdx_;
        }
    }

    void Report()
    {
        uint8_t eventCount = eventListIdx_;

        Log(P("Event Count: "), eventCount);

        if (eventCount > 1)
        {
            Event &first = eventList_[0];
            Event &last  = eventList_[eventCount - 1];

            AuditPair(first, last);

            for (uint8_t i = 1; i < eventCount; ++i)
            {
                Event &prior = eventList_[i - 1];
                Event &curr  = eventList_[i];

                AuditPair(prior, curr);
            }
        }
    }

private:

    void AuditPair(Event &prior, Event &curr)
    {
        Log(curr.eventTime - prior.eventTime,
            P(" ms: "),
            prior.eventName,
            P(" -> "),
            curr.eventName);
    }

    Event eventList_[EVENT_COUNT];

    uint8_t eventListIdx_;
};



#endif  // __DURATION_AUDITOR_H__


