#include "ShiftRegisterOutput.h"


static const uint8_t PIN_CLOCK  = 13;
static const uint8_t PIN_LATCH  = 12;
static const uint8_t PIN_SERIAL = 11;

static ShiftRegisterOut sr(PIN_CLOCK, PIN_LATCH, PIN_SERIAL);
static ShiftRegisterOutput<1> sroo(sr);


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    Write(1);
}

void Write(const uint8_t srCount)
{
    ShiftRegisterOutputPin pinList[srCount * 8];

    for (uint8_t i = 0; i < (srCount * 8); ++i)
    {
        pinList[i] = sroo.GetPin(i);
    }

    uint8_t toggleVal = 0;
    while (1)
    {
        for (uint8_t i = 0; i < (srCount * 8); ++i)
        {
            Serial.print("Pin ");
            Serial.print(i);
            Serial.print(", val: ");
            Serial.print(i % 2 ? toggleVal : !toggleVal);
            Serial.println();
            
            pinList[i].DigitalWrite(i % 2 ? toggleVal : !toggleVal);
        }

        toggleVal = !toggleVal;

        PAL.Delay(200);
    }
}

void loop() {}



