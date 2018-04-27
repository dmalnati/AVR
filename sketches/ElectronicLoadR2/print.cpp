#include <stdarg.h>
#include "print.h"
#include "Arduino.h"

void PrintInit(void)
{
    Serial1.begin(9600);
}

void print(const char *fmt, ...)
{
    // format it
    char buf[256];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end (args);

    // send it to serial
    Serial1.write(buf);
}

void sprint(char *buf, int bufSize, const char *fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    vsnprintf(buf, bufSize, fmt, args);
    va_end (args);
}

