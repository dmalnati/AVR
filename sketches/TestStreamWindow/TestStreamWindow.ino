#include "StreamWindow.h"
#include "UtlSerial.h"


void Print(StreamWindow<char> &sw)
{
    Serial.print("Cap : "); Serial.println(sw.Capacity());
    Serial.print("Size: "); Serial.println(sw.Size());
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    //
    // Here we're going to:
    // - allocate a buffer
    // - fill last element with a NULL
    // - tell stream window:
    //   - here is the buffer
    //   - it is one element smaller than we know it is
    //   - make sure to fill in any unused elements with NULL
    //
    // This gets us a c-string regardless of how full it gets
    //
    const uint8_t BUF_SIZE = 12;
    char buf[BUF_SIZE];
    buf[BUF_SIZE - 1] = '\0';

    StreamWindow<char> sw(buf, BUF_SIZE - 1, '\0');

    Serial.print("Buf size: "); Serial.println(BUF_SIZE);
    Serial.print("  buf: \""); Serial.print(buf); Serial.println("\"");
    Serial.println("Initial StreamWindow:");
    Print(sw);
    Serial.println();



    SerialShell<1> shell;

    // The "error handler" here is just to catch arbitrary input
    shell.RegisterErrorHandler([&](char *cmdStr){
        Serial.print("Got: \"");
        Serial.print(cmdStr);
        Serial.print("\", len: ");
        Serial.print(strlen(cmdStr));
        Serial.println();

        for (uint8_t i = 0; i < strlen(cmdStr); ++i)
        {
            Serial.print("Adding: \""); Serial.print(cmdStr[i]); Serial.println("\"");
            sw.Append(cmdStr[i]);
            Print(sw);
            Serial.print("  buf: \""); Serial.print(buf); Serial.println("\"");
            Serial.println();
        }

        Serial.println("Consumed");
    });

    shell.RegisterCommand("reset", [&](char *){
        Serial.println("Resetting");
        sw.Reset();
        Print(sw);
        Serial.print("  buf: \""); Serial.print(buf); Serial.println("\"");
    });

    

    shell.Run();
}

void loop() {}







