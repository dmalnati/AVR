#include "Eeprom.h"


struct SomeStruct
{
    uint8_t  a = 0;
    uint16_t b = 0;
    double   c = 0.0;
};

static SomeStruct ss;


static const uint8_t CAPACITY = 5;
static EepromAccessor<SomeStruct, CAPACITY> ea;

void PrintSomeStruct(SomeStruct &ss)
{
    @fix@Serial.print("SomeStruct { ");
    @fix@Serial.print(ss.a);
    @fix@Serial.print(", ");
    @fix@Serial.print(ss.b);
    @fix@Serial.print(", ");
    @fix@Serial.print(ss.c);
    @fix@Serial.print(" }");
    @fix@Serial.println();
}

char *ReadLine()
{
    char *retVal = NULL;
    
    const uint8_t BUF_SIZE = 100;
    static char buf[BUF_SIZE] = { 0 };

    uint8_t retValSer = @fix@Serial.readBytesUntil('\n', buf, BUF_SIZE);

    if (retValSer)
    {
        retVal = buf;
        buf[retValSer] = '\0';
    }

    return retVal;
}

void PrintOk(SomeStruct ss, uint8_t idx, uint8_t ok)
{
    if (ok) { @fix@Serial.print("OK : ["); }
    else    { @fix@Serial.print("NOK: ["); }
    
    @fix@Serial.print(idx);
    @fix@Serial.print("]: ");
    PrintSomeStruct(ss);
}

void OnLineReceived(char *line)
{
    // format
    // r x - read   at idx x
    // w x - write  at idx x
    // d x - delete at idx x
    // i x - increment values by x
    // s   - show current buffer
    // a   - show entire array

    char op = line[0];
    

    if (op == 'r')
    {
        uint8_t idx = atoi(&line[2]);
        
        @fix@Serial.print("Reading idx "); @fix@Serial.println(idx);

        SomeStruct ssTmp;
        
        uint8_t ok = ea.ReadFromIdx(ssTmp, idx);

        if (ok)
        {
            ss = ssTmp;
        }

        PrintOk(ssTmp, idx, ok);
    }
    else if (op == 'w')
    {
        uint8_t idx = atoi(&line[2]);
        @fix@Serial.print("Writing idx "); @fix@Serial.println(idx);

        uint8_t ok = ea.WriteToIdx(ss, idx);

        // Actually look at what's in there, but use success from write
        SomeStruct ssTmp;
        ea.ReadFromIdx(ssTmp, idx);

        PrintOk(ssTmp, idx, ok);
    }
    else if (op == 'd')
    {
        uint8_t idx = atoi(&line[2]);
        @fix@Serial.print("Deleting idx "); @fix@Serial.println(idx);

        uint8_t ok = ea.DeleteAtIdx(idx);

        // Actually look at what's in there, but use success from write
        SomeStruct ssTmp;
        ea.ReadFromIdx(ssTmp, idx);

        PrintOk(ssTmp, idx, ok);
    }
    else if (op == 'i')
    {
        int8_t incrBy = atoi(&line[2]);
        @fix@Serial.print("Incrementing current buffer by "); @fix@Serial.println(incrBy);

        @fix@Serial.print("Before: "); PrintSomeStruct(ss);
        ss.a += incrBy;
        ss.b += incrBy;
        ss.c += incrBy;
        @fix@Serial.print("After : "); PrintSomeStruct(ss);
    }
    else if (op == 's')
    {
        @fix@Serial.println("Showing current buffer ");
        PrintSomeStruct(ss);
    }
    else if (op == 'a')
    {
        @fix@Serial.println("Showing complete array ");

        for (uint8_t i = 0; i < ea.GetCapacity(); ++i)
        {
            SomeStruct ssTmp;
            uint8_t ok = ea.ReadFromIdx(ssTmp, i, 1);

            @fix@Serial.print(ok ? "OK : [" : "NOK: ["); @fix@Serial.print(i); @fix@Serial.print("]: ");
            PrintSomeStruct(ssTmp);
        }
    }

    @fix@Serial.println();
}

void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");
    @fix@Serial.print("Capacity: "); @fix@Serial.println(ea.GetCapacity());

    ss.a = 0;
    ss.b = 1;
    ss.c = 2.0;
    
    while (1)
    {
        char *line = ReadLine();

        if (line)
        {
            OnLineReceived(line);
        }
    }
}

void loop() {}
