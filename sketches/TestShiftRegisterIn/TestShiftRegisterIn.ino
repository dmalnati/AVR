#include "ShiftRegisterIn.h"


static const uint8_t PIN_LOAD         = 14;
static const uint8_t PIN_CLOCK        = 13;
static const uint8_t PIN_CLOCK_ENABLE = 12;
static const uint8_t PIN_SERIAL       = 11;

static ShiftRegisterIn sr(PIN_LOAD, PIN_CLOCK, PIN_CLOCK_ENABLE, PIN_SERIAL);


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    ReadAndCompare(2);
}

void ReadAndCompare(const uint8_t bufSize)
{
    uint8_t bufLast[bufSize];
    memset(bufLast, 0, bufSize);
    uint8_t buf[bufSize];
    memset(buf, 0, bufSize);

    while (1)
    {
        sr.ShiftIn(buf, bufSize);

        CompareBits(bufLast, buf, bufSize);

        memcpy(bufLast, buf, bufSize);
    }
}

void CompareBits(uint8_t *bufLast, uint8_t *buf, uint8_t bufLen)
{
    for (uint16_t i = 0; i < bufLen; ++i)
    {
        uint8_t bitmapLast = bufLast[i];
        uint8_t bitmap     = buf[i];

        if (bitmap != bitmapLast)
        {
            @fix@Serial.print("Byte "); @fix@Serial.println(i);
            
            for (uint8_t j = 0; j < 8; ++j)
            {
                uint8_t bitVal     = !!(bitmap     & (1 << (7 - j)));
                uint8_t bitValLast = !!(bitmapLast & (1 << (7 - j)));
                
                if (bitVal != bitValLast)
                {
                    @fix@Serial.print("  PISO Pin ");
                    @fix@Serial.print(7 - j);
                    @fix@Serial.print(": changed from ");
                    @fix@Serial.print(bitValLast);
                    @fix@Serial.print(" to ");
                    @fix@Serial.print(bitVal);
                    @fix@Serial.println();
                }
            }
        }
    }
}


void loop() {}


















