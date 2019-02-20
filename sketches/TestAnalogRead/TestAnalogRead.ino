#include "Log.h"
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
        Log(pinState.pin, ": ", pinState.newVal);
    }
}

void setup()
{
    LogStart(9600);
    Log("Starting");

    while (1)
    {
        for (uint8_t i = 0; i < 6; ++i)
        {
            ReportAnalogRead(pinStateList[i]);
        }
    }
}

void loop() {}



