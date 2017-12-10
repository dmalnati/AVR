#ifndef __BUFFER_FIELD_EXTRACTOR_H__
#define __BUFFER_FIELD_EXTRACTOR_H__


#include "PAL.h"


class BufferFieldExtractor
{
public:
    BufferFieldExtractor()
    : BufferFieldExtractor(NULL, 0)
    {
        // Nothing to do
    }
    
    BufferFieldExtractor(uint8_t *buf, uint8_t bufSize)
    {
        Attach(buf, bufSize);
    }
    
    void Attach(uint8_t *buf, uint8_t bufSize)
    {
        buf_     = buf;
        bufP_    = buf_;
        bufSize_ = bufSize;
    }
    
    void Reset()
    {
        bufP_ = buf_;
    }
    
    uint8_t GetUI8()
    {
        uint8_t retVal = 0;
        
        Extract(retVal);
        
        return retVal;
    }
    
    uint16_t GetUI16()
    {
        uint16_t retVal = 0;
        
        Extract(retVal);
        
        return retVal;
    }
    
    uint16_t GetUI16NTOHS()
    {
        return PAL.ntohs(GetUI16());
    }
    
private:

    template <typename T>
    uint8_t Extract(T &retVal)
    {
        uint8_t ok = 0;
        
        if (CanExtract(sizeof(T)))
        {
            ok = 1;
            
            memcpy((void *)&retVal, bufP_, sizeof(T));
            bufP_ += sizeof(T);
        }
        
        return ok;
    }

    uint8_t CanExtract(uint8_t numBytes)
    {
        return (((bufP_ + numBytes) - buf_) <= bufSize_);
    }
    
    uint8_t *buf_;
    uint8_t *bufP_;
    uint8_t  bufSize_;
};


#endif  // __BUFFER_FIELD_EXTRACTOR_H__















