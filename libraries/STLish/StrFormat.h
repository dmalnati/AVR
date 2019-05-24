#ifndef __STR_FORMAT_H__
#define __STR_FORMAT_H__


class StrFormat
{
public:
    static const uint8_t HHMMSSMMM_BUF_SIZE_NEEDED = 14;
    static const uint8_t HHMMSSMMM_IDX_MS_DOT      = 9;
    
public:

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

















