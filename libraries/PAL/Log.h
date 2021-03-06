#ifndef __LOG_H__
#define __LOG_H__


#include <float.h>

#include "PStr.h"
#include "Serial.h"


////////////////////////////////////////////////////////////////////////////////
//
// Log Init
//
////////////////////////////////////////////////////////////////////////////////

extern void LogStart(uint32_t baud);
extern void LogEnd();


////////////////////////////////////////////////////////////////////////////////
//
// Basic
//
////////////////////////////////////////////////////////////////////////////////

extern void LogNL();
extern void LogNL(uint8_t count);


////////////////////////////////////////////////////////////////////////////////
//
// Strings
//
////////////////////////////////////////////////////////////////////////////////

extern void LogNNL(const char *str);
extern void Log(const char *str);


////////////////////////////////////////////////////////////////////////////////
//
// Buffer
//
////////////////////////////////////////////////////////////////////////////////

extern void LogBufNNL(uint8_t *buf, uint16_t bufSize);
extern void LogBuf(uint8_t *buf, uint16_t bufSize);


////////////////////////////////////////////////////////////////////////////////
//
// PStr
//
////////////////////////////////////////////////////////////////////////////////

extern void LogNNL(PStr val);
extern void Log(PStr val);


////////////////////////////////////////////////////////////////////////////////
//
// Chars
//
////////////////////////////////////////////////////////////////////////////////

extern void LogNNL(char val);
extern void Log(char val);
extern void LogXNNL(char val, uint8_t count);
extern void LogX(char val, uint8_t count);



////////////////////////////////////////////////////////////////////////////////
//
// Unsigned Ints
//
////////////////////////////////////////////////////////////////////////////////

extern void LogNNL(uint32_t val);
extern void Log(uint32_t val);
extern void LogNNL(uint16_t val);
extern void Log(uint16_t val);
extern void LogNNL(uint8_t val);
extern void Log(uint8_t val);


////////////////////////////////////////////////////////////////////////////////
//
// Signed Ints
//
////////////////////////////////////////////////////////////////////////////////

extern void LogNNL(int32_t val);
extern void Log(int32_t val);
extern void LogNNL(int16_t val);
extern void Log(int16_t val);
extern void LogNNL(int8_t val);
extern void Log(int8_t val);


////////////////////////////////////////////////////////////////////////////////
//
// Number modifiers
//
////////////////////////////////////////////////////////////////////////////////

class LogBIN
{
    friend void ::LogNNL(LogBIN);
    
    struct Data
    {
        uint32_t val      = 0;
        uint32_t bitCount = 32;
    };
    
public:
    LogBIN(uint8_t val, uint8_t showPrefix = 1)
    : showPrefix_(showPrefix)
    {
        data_.val      = (uint32_t)val << 24;
        data_.bitCount = 8;
    }
    
    LogBIN(uint16_t val, uint8_t showPrefix = 1)
    : showPrefix_(showPrefix)
    {
        data_.val      = (uint32_t)val << 16;
        data_.bitCount = 16;
    }

    LogBIN(uint32_t val, uint8_t showPrefix = 1)
    : showPrefix_(showPrefix)
    {
        data_.val      = val;
        data_.bitCount = 32;
    }
    
private:
    
    void LogNNL()
    {
        if (showPrefix_)
        {
            ::LogNNL("0b");
        }
        
        for (uint8_t bitCount = 0; bitCount < data_.bitCount; ++bitCount)
        {
            uint8_t bitVal = !!((data_.val << bitCount) & 0x80000000);
            
            ::LogNNL(bitVal ? '1' : '0');
        }
    }

    Data data_;
    uint8_t showPrefix_;
};


extern void LogNNL(LogBIN val);
extern void Log(LogBIN val);


class LogHEX
{
    friend void ::LogNNL(LogHEX);
    
public:
    LogHEX(uint8_t val, uint8_t showPrefix = 1)
    : showPrefix_(showPrefix)
    , val_(val)
    {
        // Nothing to do
    }
    
private:
    
    void LogNNL()
    {
        if (showPrefix_)
        {
            ::LogNNL("0x");
        }

        uint8_t buf[2] = {
            (uint8_t)((val_ & 0xF0) >> 4),
            (uint8_t)((val_ & 0x0F) >> 0),
        };

        for (uint8_t b : buf)
        {
            if (b <= 9)
            {
                ::LogNNL(b);
            }
            else
            {
                ::LogNNL((char)('A' + (b - 10)));
            }
        }
    }

    uint8_t showPrefix_;
    uint8_t val_;
};


extern void LogNNL(LogHEX val);
extern void Log(LogHEX val);


////////////////////////////////////////////////////////////////////////////////
//
// Floating Point
//
////////////////////////////////////////////////////////////////////////////////

extern void LogNNL(double val);
extern void Log(double val);


////////////////////////////////////////////////////////////////////////////////
//
// Templates
//
////////////////////////////////////////////////////////////////////////////////

extern void Log();
extern void LogNNL();

template <typename T1, typename T2, typename ...Ts>
void LogNNL(T1 &&val1, T2 &&val2, Ts &&...args)
{
    LogNNL(val1);
    LogNNL(val2);
    LogNNL(args...);
}

template <typename T1, typename T2, typename ...Ts>
void Log(T1 &&val1, T2 &&val2, Ts &&...args)
{
    LogNNL(val1, val2, args...);
    LogNL();
}

template <typename T>
void LogXNNL(T &&val, uint8_t count)
{
    for (uint8_t i = 0; i < count; ++i)
    {
        LogNNL(val);
    }
}

template <typename T>
void LogX(T &&val, uint8_t count)
{
    LogXNNL(val, count);
    LogNL();
}




#endif  // __LOG_H__


















