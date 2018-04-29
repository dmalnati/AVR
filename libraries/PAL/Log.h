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

void LogStart(uint32_t baud)
{
    S0.Start(baud);
}

void LogEnd()
{
    S0.Stop();
}


////////////////////////////////////////////////////////////////////////////////
//
// Basic
//
////////////////////////////////////////////////////////////////////////////////

void LogNL()
{
    S0.Write('\n');
}


////////////////////////////////////////////////////////////////////////////////
//
// Strings
//
////////////////////////////////////////////////////////////////////////////////

void LogNNL(const char *str)
{
    S0.Write((uint8_t *)str, strlen(str));
}

void Log(const char *str)
{
    LogNNL(str);
    LogNL();
}


////////////////////////////////////////////////////////////////////////////////
//
// PStr
//
////////////////////////////////////////////////////////////////////////////////

void LogNNL(PStr val)
{
    const char *p = (const char *)val;
    
    uint8_t cont = 1;
    while (cont)
    {
        char c = pgm_read_byte(p);
        
        if (c != '\0')
        {
            S0.Write(c);
            
            ++p;
        }
        else
        {
            cont = 0;
        }
    }
}

void Log(PStr val)
{
    LogNNL(val);
    LogNL();
}


////////////////////////////////////////////////////////////////////////////////
//
// Chars
//
////////////////////////////////////////////////////////////////////////////////

void LogNNL(char val)
{
    S0.Write(val);
}

void Log(char val)
{
    LogNNL(val);
    LogNL();
}


////////////////////////////////////////////////////////////////////////////////
//
// Unsigned Ints
//
////////////////////////////////////////////////////////////////////////////////

void LogNNL(uint32_t val)
{
    char buf[15];
    ultoa(val, buf, 10);
    LogNNL(buf);
}

void Log(uint32_t val)
{
    LogNNL(val);
    LogNL();
}

void LogNNL(uint16_t val)
{
    LogNNL((uint32_t)val);
}

void Log(uint16_t val)
{
    Log((uint32_t)val);
}

void LogNNL(uint8_t val)
{
    LogNNL((uint32_t)val);
}

void Log(uint8_t val)
{
    Log((uint32_t)val);
}


////////////////////////////////////////////////////////////////////////////////
//
// Signed Ints
//
////////////////////////////////////////////////////////////////////////////////

void LogNNL(int32_t val)
{
    char buf[15];
    ltoa(val, buf, 10);
    LogNNL(buf);
}

void Log(int32_t val)
{
    LogNNL(val);
    LogNL();
}

void LogNNL(int16_t val)
{
    LogNNL((int32_t)val);
}

void Log(int16_t val)
{
    Log((int32_t)val);
}

void LogNNL(int8_t val)
{
    LogNNL((int32_t)val);
}

void Log(int8_t val)
{
    Log((int32_t)val);
}


////////////////////////////////////////////////////////////////////////////////
//
// Floating Point
//
////////////////////////////////////////////////////////////////////////////////

void LogNNL(double val)
{
    // Support 3 decimal places
    
    // this is -MAX_DBL, which is the largest printed number I can find:
    // 00000000011111111112222222222333333333344444
    // 12345678901234567890123456789012345678901234
    // -340282350000000000000000000000000000000.000
    
    if (isnan(val))
    {
        LogNNL(P("NaN"));
    }
    else if (isinf(val))
    {
        LogNNL(P("INF"));
    }
    else
    {
        char buf[45];
        dtostrf(val, -1, 3, buf);
        LogNNL(buf);
    }
}
    

void Log(double val)
{
    LogNNL(val);
    LogNL();
}


////////////////////////////////////////////////////////////////////////////////
//
// Templates
//
////////////////////////////////////////////////////////////////////////////////

void Log()
{
    // Nothing to do, simply here to make the templating work
}

void LogNNL()
{
    // Nothing to do, simply here to make the templating work
}

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


















