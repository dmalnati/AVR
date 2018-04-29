#include "PAL.h"
#include "Log.h"



void TestStrings()
{
    Log("Strings");
    LogX('-', 25);
    
    Log("heynow");
    LogNNL("hey"); LogNNL("NOW"); LogNL();

    LogNL();
}

void TestChars()
{
    Log("Chars");
    LogX('-', 25);
    
    char c = '!';
    Log(c);
    LogNNL(c); LogNNL(c); LogNL();
    
    LogNL();
}

void TestUnsignedInts()
{
    Log("Unsigned Ints");
    LogX('-', 25);
    
    Log((uint32_t) 0xDEADBEEF);
    LogNNL((uint32_t) 0xDEADBEEF); LogNNL((uint32_t) 0xDEADBEEF); LogNL();

    Log((uint16_t) 0xDEADBEEF);
    LogNNL((uint16_t) 0xDEADBEEF); LogNNL((uint16_t) 0xDEADBEEF); LogNL();

    Log((uint8_t) 0xDEADBEEF);
    LogNNL((uint8_t) 0xDEADBEEF); LogNNL((uint8_t) 0xDEADBEEF); LogNL();

    LogNL();
}

void TestSignedInts()
{
    Log("Signed Ints");
    LogX('-', 25);
    
    Log((int32_t) 0xDEADBEEF);
    LogNNL((int32_t) 0xDEADBEEF); LogNNL((int32_t) 0xDEADBEEF); LogNL();

    Log((int16_t) 0xDEADBEEF);
    LogNNL((int16_t) 0xDEADBEEF); LogNNL((int16_t) 0xDEADBEEF); LogNL();

    Log((int8_t) 0xDEADBEEF);
    LogNNL((int8_t) 0xDEADBEEF); LogNNL((int8_t) 0xDEADBEEF); LogNL();

    LogNL();
}

void TestFloatingPoint()
{
    Log("Floating Point");
    LogX('-', 25);


    Log((float)-1.0);
    Log((float)-0.0);
    Log((float)0.0);
    Log((float)1.0);

    Log((float)-999999999.99999);
    Log((float)999999999.99999);
    
    Log((float)-DBL_MAX);
    Log((float)DBL_MAX);

    
    Log((double)-1.0);
    Log((double)-0.0);
    Log((double)0.0);
    Log((double)1.0);

    Log((double)-999999999.99999);
    Log((double)999999999.99999);
    
    Log((double)-DBL_MAX);
    Log((double)DBL_MAX);


    LogNL();
}

void TestTemplates()
{
    Log("Templates");
    LogX('-', 25);
    
    Log();
    LogNNL();

    Log("name = ", "test");
    Log("value = ", 15);

    Log("one = ", 1, " two = ", '2');

    LogNL();
}

void setup()
{
    LogStart(9600);

    while (1)
    {
        TestStrings();
        TestChars();
        TestUnsignedInts();
        TestSignedInts();
        TestFloatingPoint();
        TestTemplates();

        LogNL();

        PAL.Delay(10000);
    }
}

void loop()
{
    
}
