#ifndef __LOG_BLOB_H__
#define __LOG_BLOB_H__


#include "Log.h"


////////////////////////////////////////////////////////////////////////////////
//
// Enhanced
//
////////////////////////////////////////////////////////////////////////////////

void LogBlob(uint8_t  *buf,
             uint16_t  bufSize,
             uint8_t   showBin = 0,
             uint8_t   showHex = 1)
{
    char bufSprintf[9];
    
    uint16_t bufSizeRemaining = bufSize;
    uint16_t byteCount        = 0;
    
    while (bufSizeRemaining)
    {
        sprintf(bufSprintf, "%08X", byteCount);
        
        // Print byte start
        LogNNL(P("0x"));
        LogNNL(bufSprintf);
        LogNNL(P(": "));
        
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
                LogNNL(bufSprintf);
                LogNNL(' ');
            }
            
            for (uint8_t i = 0; i < padBytes; ++i)
            {
                LogXNNL(' ', 3);
            }
            
            LogNNL(P("| "));
        }
        
        // Print binary
        if (showBin)
        {
            for (uint8_t i = 0; i < realBytes; ++i)
            {
                uint8_t b = buf[byteCount + i];
                
                for (uint8_t j = 0; j < 8; ++j)
                {
                    LogNNL((b & 0x80) ? 1 : 0);
                    
                    b <<= 1;
                }
                
                LogNNL(' ');
            }
            
            for (uint8_t i = 0; i < padBytes; ++i)
            {
                LogXNNL(' ', 9);
            }
            
            LogNNL(P("| "));
        }
        
        // Print visible
        for (uint8_t i = 0; i < realBytes; ++i)
        {
            char b = buf[byteCount + i];
            
            if (isprint(b))
            {
                LogNNL(b);
            }
            else
            {
                LogNNL('.');
            }
        }

        LogNL();
        
        // Account for used data
        byteCount += realBytes;
        bufSizeRemaining -= realBytes;
    }
}


#endif  // __LOG_BLOB_H__












