#include "Str.h"
#include "UtlSerial.h"


void OnLineReceived(char *line)
{
    Serial.print("line before: \""); Serial.print(line); Serial.println("\"");
    Serial.print("Buffer: "); PrintBuf();

    char delim = ' ';

    {
        Str str(line);
    
        Serial.print("Length    : "); Serial.println(str.Length());
        Serial.print("TokenCount: "); Serial.println(str.TokenCount(delim));
        Serial.print("line pre-iteration: \""); Serial.print(line); Serial.println("\"");
        Serial.print("Buffer: "); PrintBuf();
        for (uint8_t i = 0; i < str.TokenCount(delim); ++i)
        {
            Serial.print("    [");
            Serial.print(i);
            Serial.print("]: \"");
            Serial.print(str.TokenAtIdx(i, delim));
            Serial.print("\"");
            Serial.println();
        }
        Serial.print("line pre-release: \""); Serial.print(line); Serial.println("\"");
    }
    Serial.print("line post-release: \""); Serial.print(line); Serial.println("\"");
    Serial.print("Buffer: "); PrintBuf();
    Serial.println();
}


const uint8_t BUF_SIZE = 20;
char buf[BUF_SIZE];

void PrintBuf()
{
    for (uint8_t i = 0; i < BUF_SIZE; ++i)
    {
        if (buf[i])
        {
            Serial.print(buf[i]);
        }
        else
        {
            Serial.print('.');
        }
    }
    Serial.println();
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");
    
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



















