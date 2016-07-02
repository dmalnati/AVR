#include "PAL.h"
#include "Evm.h"
#include "Signal.h"

static const uint16_t BELL_202_TONE_MARK  = 1200;
static const uint16_t BELL_202_TONE_SPACE = 2200;

static const uint8_t PIN_SIGNAL_A = 27;
static const uint8_t PIN_SIGNAL_B = 28;

Pin pinSignalA(PIN_SIGNAL_A);
Pin pinSignalB(PIN_SIGNAL_B);

static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;
static IdleTimeHiResTimedEventHandlerDelegate hrted;
static Signal s;

void setup()
{
    Serial.begin(9600);

    uint16_t periodUsec = 10;

    s.SetPeriod(periodUsec);
    s.DebugSetDutyCycle(50);
    s.Start();

    
    //uint8_t tedState = 0;
    //ted.SetCallback([&](){
    //    if (!tedState) s.Stop(), tedState = 1;
    //    else s.Start(), tedState = 0;
    //});
    //ted.RegisterForTimedEventInterval(1300);

    uint8_t pct = 0;
    int8_t  dir = 1;
    hrted.SetCallback([&](){
        PAL.DigitalWrite(pinSignalB, HIGH);
        s.DebugSetDutyCycle(pct);
        PAL.DigitalWrite(pinSignalB, LOW);

        if (dir)
        {
            if (pct == 100) pct = 99, dir = 0;
            else pct += 1;
        }
        else
        {
            if (pct == 0) pct = 1, dir = 1;
            else pct -= 1;
        }
    });
    hrted.RegisterForIdleTimeHiResTimedEventInterval(10000);

    evm.MainLoop();
}

void loop() {}









