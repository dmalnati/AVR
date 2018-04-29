#include "StreamWindow.h"
#include "Utl@fix@Serial.h"


void Print(StreamWindow<char> &sw)
{
    @fix@Serial.print("Cap : "); @fix@Serial.println(sw.Capacity());
    @fix@Serial.print("Size: "); @fix@Serial.println(sw.Size());
}

void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

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

    @fix@Serial.print("Buf size: "); @fix@Serial.println(BUF_SIZE);
    @fix@Serial.print("  buf: \""); @fix@Serial.print(buf); @fix@Serial.println("\"");
    @fix@Serial.println("Initial StreamWindow:");
    Print(sw);
    @fix@Serial.println();



    SerialShell<1> shell;

    // The "error handler" here is just to catch arbitrary input
    shell.RegisterErrorHandler([&](char *cmdStr){
        @fix@Serial.print("Got: \"");
        @fix@Serial.print(cmdStr);
        @fix@Serial.print("\", len: ");
        @fix@Serial.print(strlen(cmdStr));
        @fix@Serial.println();

        for (uint8_t i = 0; i < strlen(cmdStr); ++i)
        {
            @fix@Serial.print("Adding: \""); @fix@Serial.print(cmdStr[i]); @fix@Serial.println("\"");
            sw.Append(cmdStr[i]);
            Print(sw);
            @fix@Serial.print("  buf: \""); @fix@Serial.print(buf); @fix@Serial.println("\"");
            @fix@Serial.println();
        }

        @fix@Serial.println("Consumed");
    });

    shell.RegisterCommand("reset", [&](char *){
        @fix@Serial.println("Resetting");
        sw.Reset();
        Print(sw);
        @fix@Serial.print("  buf: \""); @fix@Serial.print(buf); @fix@Serial.println("\"");
    });

    

    shell.Run();
}

void loop() {}







