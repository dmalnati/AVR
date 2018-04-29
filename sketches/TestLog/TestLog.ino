#include "PAL.h"
#include "PStr.h"
#include "LogBlob.h"
#include "Log.h"


void TestBasic()
{
    LogNL();
}

void TestStrings()
{
    Log("Strings");
    LogX('-', 25);
    
    Log("heynow");
    LogNNL("hey"); LogNNL("NOW"); LogNL();

    LogNL();
}

void TestPString()
{
    Log("PString");
    LogX('-', 25);
    
    Log(P("1234"));
    Log(P("hello world"));
    LogNNL(P("hello")); LogNNL(P(" world")); LogNL();

    auto pStr1 = P("sup1");
    Log(pStr1);

    auto pStr2 = P("sup2");
    if (!strcmp_P("sup2", pStr2))
    {
        Log("strcmp_P works");
    }
    else
    {
        Log("strcmp_P FAIL");
    }

    PStr p = P("hi guys");
    Log(p);

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

    Log((float)-NAN);
    Log((float)NAN);
    Log((float)-INFINITY);
    Log((float)INFINITY);

    
    Log((double)-1.0);
    Log((double)-0.0);
    Log((double)0.0);
    Log((double)1.0);

    Log((double)-999999999.99999);
    Log((double)999999999.99999);
    
    Log((double)-DBL_MAX);
    Log((double)DBL_MAX);

    Log((double)-NAN);
    Log((double)NAN);
    Log((double)-INFINITY);
    Log((double)INFINITY);


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

    Log("tomato", " ", P("tomato"));

    LogNL();
}

void TestEnhanced()
{
    char buf[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "abcdefghijklmnopqrstuvwxyz"
                 "0123456789";

    LogBlob((uint8_t *)buf, sizeof(buf), 1, 1);
}


void setup()
{
    LogStart(9600);

    while (1)
    {
        TestBasic();
        TestStrings();
        TestPString();
        TestChars();
        TestUnsignedInts();
        TestSignedInts();
        TestFloatingPoint();
        TestTemplates();
        TestEnhanced();

        LogNL();

        PAL.Delay(10000);
    }
}

void loop()
{
    
}
