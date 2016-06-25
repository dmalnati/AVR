#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "UtlStreamBlob.h"


void StreamBlob(Print    &stream,
                uint8_t  *buf,
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
        stream.print("0x");
        stream.print(bufSprintf);
        stream.print(": ");
        
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
                stream.print(bufSprintf);
                stream.print(" ");
            }
            
            for (uint8_t i = 0; i < padBytes; ++i)
            {
                stream.print("   ");
            }
            
            stream.print("| ");
        }
        
        // Print binary
        if (showBin)
        {
            for (uint8_t i = 0; i < realBytes; ++i)
            {
                uint8_t b = buf[byteCount + i];
                
                for (uint8_t j = 0; j < 8; ++j)
                {
                    stream.print((b & 0x80) ? 1 : 0);
                    
                    b <<= 1;
                }
                
                stream.print(" ");
            }
            
            for (uint8_t i = 0; i < padBytes; ++i)
            {
                stream.print("         ");
            }
            
            stream.print("| ");
        }
        
        // Print visible
        for (uint8_t i = 0; i < realBytes; ++i)
        {
            uint8_t b = buf[byteCount + i];
            
            if (isprint(b))
            {
                stream.write(b);
            }
            else
            {
                stream.print('.');
            }
        }

        stream.println();
        
        // Account for used data
        byteCount += realBytes;
        bufSizeRemaining -= realBytes;
    }
}


















