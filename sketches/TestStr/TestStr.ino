#include "Str.h"
#include "Utl@fix@Serial.h"


void OnLineReceived(char *line)
{
    @fix@Serial.print("line before: \""); @fix@Serial.print(line); @fix@Serial.println("\"");
    @fix@Serial.print("Buffer: "); PrintBuf();

    char delim = ' ';

    {
        Str str(line);
    
        @fix@Serial.print("Length    : "); @fix@Serial.println(str.Length());
        @fix@Serial.print("TokenCount: "); @fix@Serial.println(str.TokenCount(delim));
        @fix@Serial.print("line pre-iteration: \""); @fix@Serial.print(line); @fix@Serial.println("\"");
        @fix@Serial.print("Buffer: "); PrintBuf();
        for (uint8_t i = 0; i < str.TokenCount(delim); ++i)
        {
            @fix@Serial.print("    [");
            @fix@Serial.print(i);
            @fix@Serial.print("]: \"");
            @fix@Serial.print(str.TokenAtIdx(i, delim));
            @fix@Serial.print("\"");
            @fix@Serial.println();
        }
        @fix@Serial.print("line pre-release: \""); @fix@Serial.print(line); @fix@Serial.println("\"");
    }
    @fix@Serial.print("line post-release: \""); @fix@Serial.print(line); @fix@Serial.println("\"");
    @fix@Serial.print("Buffer: "); PrintBuf();
    @fix@Serial.println();
}


const uint8_t BUF_SIZE = 20;
char buf[BUF_SIZE];

void PrintBuf()
{
    for (uint8_t i = 0; i < BUF_SIZE; ++i)
    {
        if (buf[i])
        {
            @fix@Serial.print(buf[i]);
        }
        else
        {
            @fix@Serial.print('.');
        }
    }
    @fix@Serial.println();
}

void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");
    
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



















