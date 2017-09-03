#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "UtlStreamBlob.h"


void StreamBlob(uint8_t  *buf,
                uint16_t  bufSize,
                uint8_t   showBin,
                uint8_t   showHex)
{
    char bufSprintf[9];
    
    uint16_t bufSizeRemaining = bufSize;
    uint16_t byteCount        = 0;
    
    while (bufSizeRemaining)
    {
        sprintf(bufSprintf, "%08X", byteCount);
        
        // Print byte start
        printf("0x");
        printf(bufSprintf);
        printf(": ");
        
        // Calculate how many real vs pad bytes to output
        uint8_t realBytes = (byteCount + 8 > bufSize) ?
                            (bufSize - byteCount)     :
                            8;
        uint8_t padBytes = 8 - realBytes;
        
        // Print hex
        if (showHex)
        {
            for (uint8_t i = 0; i < realBytes; ++i)
            {
                uint8_t b = buf[byteCount + i];
                
                sprintf(bufSprintf, "%02X", b);
                printf(bufSprintf);
                printf(" ");
            }
            
            for (uint8_t i = 0; i < padBytes; ++i)
            {
                printf("   ");
            }
            
            printf("| ");
        }
        
        // Print binary
        if (showBin)
        {
            for (uint8_t i = 0; i < realBytes; ++i)
            {
                uint8_t b = buf[byteCount + i];
                
                for (uint8_t j = 0; j < 8; ++j)
                {
                    printf((b & 0x80) ? "1" : "0");
                    
                    b <<= 1;
                }
                
                printf(" ");
            }
            
            for (uint8_t i = 0; i < padBytes; ++i)
            {
                printf("         ");
            }
            
            printf("| ");
        }
        
        // Print visible
        for (uint8_t i = 0; i < realBytes; ++i)
        {
            uint8_t b = bufSprintf[byteCount + i];
            
            if (isprint(b))
            {
                bufSprintf[0] = b;
                bufSprintf[1] = '\0';
                printf(bufSprintf);
                //printf(b);
            }
            else
            {
                printf(".");
            }
        }

        printf("\r\n");
        
        // Account for used data
        byteCount += realBytes;
        bufSizeRemaining -= realBytes;
    }
}


















