#include "PAL.h"



struct PinState
{
    uint8_t  pin;
    uint16_t oldVal;
    uint16_t newVal;
};

static PinState pinStateList[] = {
    { 23, 0, 0 },
    { 24, 0, 0 },
    { 25, 0, 0 },
    { 26, 0, 0 },
    { 27, 0, 0 },
    { 28, 0, 0 }
};

void ReportAnalogRead(PinState &pinState)
{
    // move prior newVal to oldVal
    pinState.oldVal = pinState.newVal;

    // do read
    pinState.newVal = PAL.AnalogRead(pinState.pin);

    // decide if value changed
    if (pinState.oldVal != pinState.newVal)
    {
        @fix@Serial.print(pinState.pin);
        @fix@Serial.print(": ");
        @fix@Serial.println(pinState.newVal);
    }
}

void setup()
{
    @fix@Serial.begin(9600);

    while (1)
    {
        for (uint8_t i = 0; i < 6; ++i)
        {
            ReportAnalogRead(pinStateList[i]);
        }
    }
}

void loop() {}



