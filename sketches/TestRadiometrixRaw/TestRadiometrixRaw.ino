#include "PAL.h"

static const uint8_t PIN_PWM = 15;

void TestStripe()
{
    uint8_t val = 0;
    
    while (1)
    {
        PAL.AnalogWrite(PIN_PWM, val);

        PAL.Delay(10);

        ++val;
    }
}

void TestFlipFlop()
{
    while (1)
    {
        PAL.AnalogWrite(PIN_PWM, 0);

        PAL.Delay(1000);

        PAL.AnalogWrite(PIN_PWM, 255);

        PAL.Delay(1000);
    }
}

void TestTone()
{
    uint8_t pinArduino = PAL.GetArduinoPinFromPhysicalPin(PIN_PWM);

    static const uint16_t BELL_202_TONE_MARK  = 1200;
    static const uint16_t BELL_202_TONE_SPACE = 2200;
    
    while (1)
    {
        tone(pinArduino, BELL_202_TONE_MARK);

        PAL.Delay(1000);

        tone(pinArduino, BELL_202_TONE_SPACE);

        PAL.Delay(1000);
    }
}

void setup()
{
    TestTone();
}
 
void loop() { }


