#ifndef __SIMPLE_STRING_H__
#define __SIMPLE_STRING_H__


template <uint8_t COUNT_CHARS>
class SimpleString
{
public:
    SimpleString()
    {
        Clear();
    }
    
    void Clear()
    {
        buf_[0] = '\0';
        bufLen_ = 0;
    }
    
    uint8_t Append(char c)
    {
        uint8_t retVal = 0;
        
        if (bufLen_ < COUNT_CHARS)
        {
            retVal = 1;
            
            buf_[bufLen_] = c;
            ++bufLen_;
            
            buf_[bufLen_] = '\0';
        }
        
        return retVal;
    }
    
    uint8_t Length()
    {
        return bufLen_;
    }
    
    char operator[](uint8_t i)
    {
        return buf_[i];
    }
    
    char *UnsafePtr()
    {
        return buf_;
    }
    
private:
    char    buf_[COUNT_CHARS + 1];
    uint8_t bufLen_;
};


#endif  // __SIMPLE_STRING_H__