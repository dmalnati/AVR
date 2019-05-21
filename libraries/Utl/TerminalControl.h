#ifndef __TERMINAL_CONTROL_H__
#define __TERMINAL_CONTROL_H__


#include "Log.h"


class TerminalControl
{
public:

    enum class Color : uint8_t
    {
        BLACK = 0,
        RED,
        GREEN,
        YELLOW,
        BLUE,
        MAGENTA,
        CYAN,
        WHITE,
    };
    
    static void ChangeColor(Color c)
    {
        // The space is the character we need to determine from the color
        char buf[] = "\u001b[3 m";
        
        // Set based on enum value, as ascii char
        buf[3] = '0' + (uint8_t)c;
        
        LogNNL(buf);
    }
};


#endif  // __TERMINAL_CONTROL_H__












