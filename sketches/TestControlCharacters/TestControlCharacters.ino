#include "Log.h"
#include "PAL.h"


// https://en.wikipedia.org/wiki/ANSI_escape_code
// http://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html


const char *Green()
{
    return "\u001b[32m";
}

const char *Normal()
{
    return "\u001b[37m";
}

const char *Red()
{
    return "\u001b[31m";
}


void setup()
{
    LogStart(9600);
    Log("Starting");

    while (1)
    {

        LogNNL(Green());
        Log("Green?");

        LogNNL(Normal());
        Log("Normal?");

        LogNNL(Red());
        Log("Red?");

        LogNL();
        PAL.Delay(1000);
    }
}

void loop() {}
