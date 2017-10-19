#ifndef __STR_FORMAT_H__
#define __STR_FORMAT_H__


void U32ToStrPadLeft(char *bufTarget, uint32_t val, uint8_t width, char pad)
{
    // Allocate local space so the NULL terminator doesn't spill beyond the
    // width specified in the target buffer.
    char buf[width + 1];
 
    // Convert to ASCII representation, plus NULL, base 10
    ultoa(val, buf, 10);
   
    // Shift ASCII chars to the right and pad if necessary
    uint8_t len = strlen(buf);
   
    if (len < width)
    {
        memmove(&buf[width - len], buf, len + 1);
        memset(buf, pad, width - len);
    }
   
    // Copy chars (not null terminator) to target buffer
    memcpy(bufTarget, buf, width);
}


#endif  // __STR_FORMAT_H__

















