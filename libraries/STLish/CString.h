#ifndef __CSTRING_H__
#define __CSTRING_H__


#include <string.h>


// Operates on unowned memory
class CString
{
public:

    CString(char *str, uint8_t capacity)
    : str_(str)
    , capacity_(capacity)
    , c_('\0')
    {
        // Nothing to do
    }
    
    char &operator[](uint8_t i)
    {
        if (str_ && i < capacity_)
        {
            return str_[i];
        }
        else
        {
            c_ = '\0';
            
            return c_;
        }
    }
    
    uint8_t Length()
    {
        uint8_t retVal = 0;
        
        if (str_)
        {
            retVal = strlen(str_);
        }
        
        return retVal;
    }
    
    void Copy(const char *str)
    {
        if (str_)
        {
            strncpy(str_, str, capacity_ - 1);
            
            str_[capacity_ - 1] = '\0';
        }
    }
    
    void ToUpper()
    {
        if (str_)
        {
            uint8_t strLen = strlen(str_);
            
            for (uint8_t i = 0; i < strLen; ++i)
            {
                str_[i] = toupper(str_[i]);
            }
        }
    }
    
    uint8_t Prepend(char c)
    {
        uint8_t retVal = 0;
        
        if (str_)
        {
            uint8_t strLen = Length();
            
            if (strLen < capacity_ - 1)
            {
                retVal = 1;
                
                // shift everything right
                for (uint8_t i = strLen; i > 0; --i)
                {
                    str_[i] = str_[i - 1];
                }
                
                str_[0] = c;
            }
        }
        
        return retVal;
    }
    
    void PadRight(char c)
    {
        if (str_)
        {
            for (uint8_t i = Length(); i < capacity_ - 1; ++i)
            {
                str_[i] = c;
            }
        }
    }

    
private:

    char    *str_;
    uint8_t  capacity_;
    
    char c_;
};


#endif  // __CSTRING_H__




