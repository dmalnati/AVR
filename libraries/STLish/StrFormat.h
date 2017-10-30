#ifndef __STR_FORMAT_H__
#define __STR_FORMAT_H__


void U32ToStrPadLeft(char *bufTarget, uint32_t val, uint8_t width, char pad)
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


#endif  // __STR_FORMAT_H__

















