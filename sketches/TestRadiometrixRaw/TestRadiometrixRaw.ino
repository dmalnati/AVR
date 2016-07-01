#include "PAL.h"
#include "Evm.h"
#include "LedFader.h"

static const uint8_t PIN_PWM = 15;

void TestStripe()
{
    uint8_t val = 0;
    
    while (1)
    {
        //PAL.AnalogWrite(PIN_PWM, val);

        PAL.Delay(10);

        ++val;
    }
}

void TestFlipFlop()
{
    while (1)
    {
        //PAL.AnalogWrite(PIN_PWM, 0);

        PAL.Delay(1000);

        //PAL.AnalogWrite(PIN_PWM, 255);

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

void TestFader()
{
    LEDFader<1,1> fader1;
    Evm::Instance<10,10,10> evm;

    fader1.AddLED(PIN_PWM);

    while (1)
    {
        fader1.FadeOnce();

        evm.HoldStackDangerously(1000);
    }
}

void TestMaxToggle()
{
    Pin p(PIN_PWM);

    while (1)
    {
        PAL.DigitalWrite(p, LOW);
        delayMicroseconds(1);
        PAL.DigitalWrite(p, HIGH);
        delayMicroseconds(1);
    }
}

void TestDelayResolution()
{
    uint16_t baud = 1200;
    uint16_t usDelay = (uint16_t)((1000.0 / (double)baud) * 1000.0);

    // usDelay = 833 (from 833.33333....)

    Pin p(PIN_PWM);
    
    while (1)
    {
        PAL.DigitalWrite(p, LOW);
        delayMicroseconds(usDelay);
        PAL.DigitalWrite(p, HIGH);
        delayMicroseconds(usDelay);
    }
}

// this is worse than delayMicroseconds
void MyDelay(uint16_t usDelay)
{
    uint32_t start = micros();
    while (micros() - start < usDelay) { }
}

void setup()
{
    //TestFader();
    //TestTone();
    //TestMaxToggle();

    TestDelayResolution();
}
 
void loop() { }


