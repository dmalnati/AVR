#ifndef __RTC_H__
#define __RTC_H__


#include "PAL.h"
#include "StrFormat.h"


/*
 * On 8MHz AVR test program, was seen to be approx 1s fast for each minute.
 */
class RTC
{
    static const uint32_t MS_PER_HOUR   = 60 * 60 * 1000L;
    static const uint32_t MS_PER_MINUTE =      60 * 1000L;
    static const uint32_t MS_PER_SECOND =           1000L;

public:
    RTC()
    : hour_(0)
    , minute_(0)
    , second_(0)
    , ms_(0)
    , timeAtLastSync_(PAL.Millis())
    {
        // Nothing to do
    }

    void Sync(uint8_t hour, uint8_t minute, uint8_t second, uint32_t ms)
    {
        if (hour != hour_ || minute != minute_ || second != second_ || ms != ms_)
        {
            hour_   = hour;
            minute_ = minute;
            second_ = second;
            ms_     = ms;

            timeAtLastSync_ = PAL.Millis();
        }
    }
    
    struct Time
    {
        uint8_t  hour   = 0;
        uint8_t  minute = 0;
        uint8_t  second = 0;
        uint32_t ms     = 0;
    };
    
    Time GetTime()
    {
        // Calculate duration since last sync
        uint32_t timeNow = PAL.Millis();
        
        uint32_t timeDiffMs = timeNow - timeAtLastSync_;
        
        uint32_t timeDiffRemainingMs = timeDiffMs;
        
        // Calculate time that has passed            
        uint8_t hourOffset = timeDiffRemainingMs / MS_PER_HOUR;
        timeDiffRemainingMs -= (hourOffset * MS_PER_HOUR);
        
        uint8_t minuteOffset = timeDiffRemainingMs / MS_PER_MINUTE;
        timeDiffRemainingMs -= (minuteOffset * MS_PER_MINUTE);
        
        uint8_t secondOffset = timeDiffRemainingMs / MS_PER_SECOND;
        timeDiffRemainingMs -= (secondOffset * MS_PER_SECOND);
        
        uint32_t msOffset = timeDiffRemainingMs;
        
        
        // Add time to last known sync'd clock
        t_.hour   = hour_;
        t_.minute = minute_;
        t_.second = second_;
        t_.ms     = ms_;
        
        t_.ms += msOffset;
        if (t_.ms >= 1000L)
        {
            secondOffset += 1;
            
            t_.ms -= 1000L;
        }
        
        t_.second += secondOffset;
        if (t_.second >= 60)
        {
            minuteOffset += 1;
            
            t_.second -= 60;
        }
        
        t_.minute += minuteOffset;
        if (t_.minute >= 60)
        {
            hourOffset += 1;
            
            t_.minute -= 60;
        }
        
        t_.hour += hourOffset;
        if (t_.hour >= 24)
        {
            t_.hour -= 24;
        }
        
        return t_;
    }
    
    char *GetTimeAsString()
    {
        GetTime();
        
        char *p = buf_;
        
        // HH
        U32ToStrPadLeft(p, t_.hour, 2, '0');
        p += 2;
        
        // :
        *p = ':';
        p += 1;
        
        // MM
        U32ToStrPadLeft(p, t_.minute, 2, '0');
        p += 2;
        
        // :
        *p = ':';
        p += 1;
        
        // SS
        U32ToStrPadLeft(p, t_.second, 2, '0');
        p += 2;
        
        // .
        *p = '.';
        p += 1;
        
        // mmm
        U32ToStrPadLeft(p, t_.ms, 3, '0');
        p += 3;
        
        // NULL
        *p = '\0';
        
        return buf_;
    }

private:
    uint8_t  hour_;
    uint8_t  minute_;
    uint8_t  second_;
    uint32_t ms_;
    
    Time t_;
    
    // HH:MM:SS.mmm = 12 bytes + 1 NULL = 13
    static const uint8_t BUF_SIZE = 13;
    
    char buf_[BUF_SIZE];
    
    uint32_t timeAtLastSync_;
};


#endif  // __RTC_H__