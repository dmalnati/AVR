#ifndef __STR_H__
#define __STR_H__


// Class to operate on an unowned string.
// Will modify string while in use.
// Will restore string to original state after destruction.


class Str
{
public:
    Str(char *str = NULL)
    : str_(NULL)
    , len_(0)
    , delimLast_('\0')
    {
        Attach(str);
    }
    
    ~Str()
    {
        Release();
    }
    
    void Attach(char *str)
    {
        Release();
        
        str_ = str;
        if (str_)
        {
            len_ = strlen(str);
        }
    }
    
    void Release()
    {
        RestoreOriginalString();
        
        str_ = NULL;
        len_ = 0;
    }
    
    uint8_t Length()
    {
        return len_;
    }
    
    uint8_t TokenCount(char delim)
    {
        RestoreOriginalString();
        delimLast_ = delim;
        
        uint8_t retVal = 0;
        
        if (str_)
        {
            const char  delimList[2] = { delim, '\0' };
            char       *tokenStr     = str_;
            char       *token        = NULL;
            
            token = strtok(tokenStr, delimList);
            
            while (token)
            {
                ++retVal;
                
                token = strtok(NULL, delimList);
            }
        }
        
        return retVal;
    }
    
    char *TokenAtIdx(uint8_t idx, char delim)
    {
        char *retVal = NULL;
        
        if (str_)
        {
            uint8_t tokenCount = TokenCount(delim);
            RestoreOriginalString();
            delimLast_ = delim;
            
            if (idx < tokenCount)
            {
                const char  delimList[2] = { delim, '\0' };
                char       *tokenStr     = str_;
                char       *token        = NULL;
                uint8_t     atIdx        = 0;
                
                token = strtok(tokenStr, delimList);
                
                while (token)
                {
                    if (atIdx == idx)
                    {
                        // note the start location of the string
                        // note the index of the NULL
                        char *subStr     = token;
                        char *subStrNull = subStr + strlen(subStr);
                        
                        // wind to end of string
                        while ((token = strtok(NULL, delimList))) { }
                        
                        // place null even though we expect it to be there.
                        // now we don't have to hope, we can guarantee.
                        // basically this isn't a guaranteed behavior and we're
                        // forcing a particular implementation.
                        *subStrNull = '\0';
                        
                        // store pointer to substring to return
                        retVal = subStr;
                    }
                    else
                    {
                        token = strtok(NULL, delimList);
                    }
                    
                    ++atIdx;
                }
            }
        }
        
        return retVal;
    }

private:

    void RestoreOriginalString()
    {
        // this strtok implementation leaves nulls in the string.
        // we don't even need to care how many, just get rid of all
        // of them before the end of the string.
        
        for (uint8_t i = 0; i < len_; ++i)
        {
            if (str_[i] == '\0')
            {
                str_[i] = delimLast_;
            }
        }
        
        delimLast_ = '\0';
    }

    char    *str_;
    uint8_t  len_;
    char     delimLast_;
};



#endif  // __STR_H__























