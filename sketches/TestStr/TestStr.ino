#include "Log.h"
#include "Str.h"
#include "SerialInput.h"


void OnLineReceived(char *line)
{
    Log("line before: \"", line, "\"");
    LogNNL("Buffer: "); PrintBuf();

    char delim = ' ';

    {
        Str str(line);
    
        Log("Length    : ", str.Length());
        Log("TokenCount: ", str.TokenCount(delim));
        Log("line pre-iteration: \"", line, "\"");
        LogNNL("Buffer: "); PrintBuf();
        for (uint8_t i = 0; i < str.TokenCount(delim); ++i)
        {
            Log("    [", i, "]: \"", str.TokenAtIdx(i, delim), "\"");
        }
        Log("line pre-release: \"", line, "\"");
    }
    Log("line post-release: \"", line, "\"");
    LogNNL("Buffer: "); PrintBuf();
    LogNL();
}


const uint8_t BUF_SIZE = 20;
char buf[BUF_SIZE];

void PrintBuf()
{
    for (uint8_t i = 0; i < BUF_SIZE; ++i)
    {
        if (buf[i])
        {
            LogNNL(buf[i]);
        }
        else
        {
            LogNNL('.');
        }
    }
    LogNL();
}

void setup()
{
    LogStart(9600);
    Log("Starting");
    
    while (1)
    {   
        char lineLen = SerialReadLine(buf, BUF_SIZE);

        if (lineLen)
        {
            OnLineReceived(buf);
        }
    }
}

void loop() {}



















