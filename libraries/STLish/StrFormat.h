#ifndef __STR_FORMAT_H__
#define __STR_FORMAT_H__


#include <string.h>
#include <stdint.h>
#include <stdlib.h>


class StrFormat
{
public:
    static const uint8_t HHMMSSMMM_BUF_SIZE_NEEDED = 14;
    static const uint8_t HHMMSSMMM_IDX_MS_DOT      = 9;
    static const uint8_t COMMAS_BUF_SIZE_NEEDED_32 = 14;

public:

    // does not attempt to null terminate.
    // simply slots the string into the target.
    static void U32ToStrPadLeft(char *bufTarget, uint32_t val, uint8_t width, char pad)
    {
        // Deal with buffer needed to hold a U32 plus NULL
        static const uint8_t U32_MAX_STR_LEN     = 10;
        static const uint8_t U32_TO_STR_BUF_SIZE = U32_MAX_STR_LEN + 1;
       
        char u32StrBuf_[U32_TO_STR_BUF_SIZE];
        
        // Convert to ASCII representation, plus NULL, base 10
        ultoa(val, u32StrBuf_, 10);
       
        // Determine actual length of numeric string
        uint8_t len = strlen(u32StrBuf_);
        
        // Calculate number of pad bytes and number of bytes to copy
        // given that it's both possible the destination buffer is smaller than
        // necessary to fit the entire thing, or that it's larger than the
        // numeric string.
        uint8_t padBytes;
        uint8_t bytesToCopy;
        
        if (width <= len)
        {
            padBytes    = 0;
            bytesToCopy = width;
        }
        else
        {
            padBytes    = width - len;
            bytesToCopy = len;
        }
        
        // Actually do some copying
        memset(bufTarget, pad, padBytes);
        memcpy(&bufTarget[padBytes], u32StrBuf_, bytesToCopy);
    }

    // requires a 14 byte buf total, not required to be NULL terminated yet
    static char *U32ToStrCommasPadLeft(char *bufTarget, uint32_t val)
    {
        U32ToStrCommas(bufTarget, val);
        return ShiftRightAddNull(bufTarget, 14);
    }


    // 14 byte buffer required.
    // Buffer not required to be pre-null-terminated.
    //
    // Converts integer to comma-grouped string, no padding.
    //    4294967295
    // 4,294,967,295 = 14 bytes required -- 13 bytes plus null terminator required as bufTarget size
    //  | 1         = idx comma
    //      | 5     = idx comma
    //          | 9 = idx comma 
    //
    static char *U32ToStrCommas(char *bufTarget, uint32_t val)
    {
        const uint8_t MIN_BUF_WIDTH = 13;
        U32ToStrPadLeft(bufTarget, val, MIN_BUF_WIDTH, ' ');

        // Now we have a buffer which may be left-padded, so we can find places
        // to insert a comma and shift everything else left.
        for (auto commaIdx : (uint8_t[]){ 9, 5, 1 })
        {
            if (bufTarget[commaIdx] != ' ')
            {
                // shift everything left leaving a place for the comma
                for (uint8_t i = 0; i < commaIdx; ++i)
                {
                    bufTarget[i] = bufTarget[i + 1];
                }

                // add the comma
                bufTarget[commaIdx] = ',';
            }
        }

        // Find where padding ends
        uint8_t idxValStart = GetPaddingEndIdx(bufTarget, MIN_BUF_WIDTH);

        // Shift everything left
        for (uint8_t i = 0; i < idxValStart; ++i)
        {
            bufTarget[i] = bufTarget[i + idxValStart];
        }

        // Add null terminator
        bufTarget[MIN_BUF_WIDTH - idxValStart] = '\0';

        // Make it easier to use inline in functions
        return bufTarget;
    }

    // returns the index of the first byte which isn't a space
    static uint8_t GetPaddingEndIdx(char *buf, uint8_t byteCount)
    {
        uint8_t idxValStart = 0;

        uint8_t found = 0;
        for (uint8_t i = 0; i < byteCount && !found; ++i)
        {
            if (buf[i] != ' ')
            {
                idxValStart = i;
                found = 1;
            }
        }

        return idxValStart;
    }

    // Assumes the space given has 1 byte accounted for in the
    // byteCount for NULL at end.
    //
    // Assumes this is a NULL terminated string already, within the
    // bytes being handled.  Valid to be using the full capacity already.
    //
    // This code will ensure NULL is added.
    // This code operates within byteCount-1 for shifting, then adds
    // NULL at the end.
    //
    // Empty space left from shifting is padded with spaces.
    static char *ShiftRightAddNull(char *bufTarget, uint32_t byteCount)
    {
        uint8_t lenNow   = strlen(bufTarget);
        uint8_t lenLater = byteCount - 1;

        uint8_t diff = lenLater - lenNow;

        for (uint8_t i = 0; i < lenNow; ++i)
        {
            bufTarget[lenLater - 1 - i] = bufTarget[lenLater - 1 - (i + diff)];
        }

        for (uint8_t i = 0; i < diff; ++i)
        {
            bufTarget[i] = ' ';
        }

        bufTarget[lenLater] = '\0';

        return bufTarget;
    }


    // buf has to be:
    // HHH:MM:SS.mmm = 14 bytes = 13 chars + 1 byte terminator
    static void DurationMsToHHMMSSMMM(uint32_t durationMs, char *buf)
    {
        constexpr int32_t ONE_HOUR_IN_MS = 60UL * 60UL * 1000UL; // 3,600,000
        constexpr int32_t ONE_MIN_IN_MS  =        60UL * 1000UL; //    60,000
        constexpr int32_t ONE_SEC_IN_MS  =               1000UL; //     1,000

        // Working with signed integers because thats what the ldiv function works with.
        // We pass around unsigned uint32_t typically, but we'll move it to
        // be an int32_t.
        //
        // That cuts its max value in half.
        //
        // Max value is now 2^31-1 = 2,147,483,647
        // Max number of values is 2^31
        //
        // Max number of hours-of-ms that can fit in that:
        // 2,147,483,648 / 3,600,000 = 596 (truncated)

        // convert unsigned to signed, good luck everybody
        int32_t msRemaining = (int32_t)durationMs;

        ldiv_t qrLong;

        // Calculate number of hours
        qrLong = ldiv(msRemaining, ONE_HOUR_IN_MS);
        uint32_t hours = qrLong.quot;
        msRemaining = qrLong.rem;

        // Calculate number of minutes
        qrLong = ldiv(msRemaining, ONE_MIN_IN_MS);
        uint32_t minutes = qrLong.quot;
        msRemaining = qrLong.rem;

        // Calculate number of seconds
        qrLong = ldiv(msRemaining, ONE_SEC_IN_MS);
        uint32_t seconds = qrLong.quot;
        msRemaining = qrLong.rem;

        // Actually format it as a duration string
        U32ToStrPadLeft(&buf[0], hours, 3, ' ');
        buf[3] = ':';
        U32ToStrPadLeft(&buf[4], minutes, 2, '0');
        buf[6] = ':';
        U32ToStrPadLeft(&buf[7], seconds, 2, '0');
        buf[9] = '.';
        U32ToStrPadLeft(&buf[10], msRemaining, 3, '0');
        buf[13] = '\0';
    }
    
    static void TrimMsFromHHMMSSMMM(char *buf)
    {
        buf[HHMMSSMMM_IDX_MS_DOT] = '\0'; 
    }
    
    // 0:30:00.000 becomes
    //   30:00.000
    // 0:00:00.500 becomes
    //       0.500
    // 0:00:01.500 becomes
    //       1.500
    // 0:00:00     becomes
    //       0
    static void TrimLeadingPaddingFromHHMMSSMMM(char *buf)
    {
        uint8_t len = strlen(buf);
        
        uint8_t cont = 1;
        uint8_t i;
        for (i = 0; i < len && cont; ++i)
        {
            char c = buf[i];
            
            // Search for leading useless time formatting padding zeros.
            //
            if (c != '0' && c != ':' && c != ' ')
            {
                // We found something significant, so we're stopping 
                cont = 0;
                
                if (c == '.')
                {
                    buf[i - 1] = '0';
                }
            }
            else
            {
                buf[i] = ' ';
            }
        }
        
        if (i == len && cont)
        {
            buf[len - 1] = '0';
        }
    }
};


#endif  // __STR_FORMAT_H__

















