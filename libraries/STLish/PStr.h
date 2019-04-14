#ifndef __PSTR_H__
#define __PSTR_H__


#include <avr/pgmspace.h>


// See here for details around PSTR and F macros
// http://forum.arduino.cc/index.php?topic=91314.0
//
// Basically:
// - PSTR is the AVR macro, helps make progmem strings.
// - F is Arduino, casts the "regular" pointer returned by PSTR into a
//   type the print function knows to cast back and use progmem functions on.


// Use cases I care about and want to address in this file:
// - Create a progmem string and hold onto it for later
//   - string compares
//   - printing
// - Create a progmem string on the fly
//   - print directly


// So create a P macro which satisfies all of the above.

// You can stop using F completely.
// You can stop using PSTR too.
// Just use P.


#define P(str) (PStr{PSTR(str)})


//
// Meant to work with
// https://www.nongnu.org/avr-libc/user-manual/group__avr__pgmspace.html
//
class PStr
{
public:
    PStr()
    : PStr(GetDefaultValue())
    {
        // Nothing to do
    }

    PStr(const char *p)
    : p_(p)
    {
        // Nothing to do
    }
    
    operator const char *()
    {
        return p_;
    }
    
    operator const void *()
    {
        return p_;
    }

private:
    const char *p_;
    
    static const char *GetDefaultValue()
    {
        static PStr pStr = P("");
        
        return (const char *)pStr;
    }
};




#endif  // __PSTR_H__



















