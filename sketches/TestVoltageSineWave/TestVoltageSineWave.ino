#include "PAL.h"

const uint16_t SIN_STEPS = 512;

uint8_t sinTable[SIN_STEPS] = { 0 };

void CalculateSinSteps()
{
    for (uint32_t i = 0; i < SIN_STEPS; ++i)
    {
        sinTable[i] = 127 + 127*sin(2.0 * PI * ((double)i / (double)SIN_STEPS));
    }
}

void SetupOutputPins()
{
    // Set up pins of PORTD as output
    for (uint8_t p = 4; p < 4 + 11; ++p)
    {
        PAL.PinMode(p, OUTPUT);
    }

    PAL.PinMode(15, OUTPUT);
}

void OutputSignal()
{
    uint16_t sinTableIdx = 0;
    
    while (1)
    {
        PORTD = sinTable[sinTableIdx];

        sinTableIdx = (sinTableIdx + 1) % SIN_STEPS;

        //delayMicroseconds(1);
    }
}

void Debug()
{
    // at 40kHz, it's 25us per sample
    
    // 19us to toggle up then down
    for (uint8_t i = 0; i < 100; ++i)
    {
        PAL.DigitalToggle(15);
    }
}


void setup()
{
    SetupOutputPins();

    Debug();

    CalculateSinSteps();

    OutputSignal();
}



void loop() {}




