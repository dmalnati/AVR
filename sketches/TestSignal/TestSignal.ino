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


static const uint8_t SINE_WAVE_STEP_COUNT = 8;
static SignalSourceSineWave<SINE_WAVE_STEP_COUNT> sineWave;
//static SignalDAC s(&sineWave);

void setup()
{
    Serial.begin(9600);

    PAL.PinMode(pinSignalA, OUTPUT);
    PAL.PinMode(pinSignalB, OUTPUT);

    //uint16_t periodUsec = 10000;
    //s.SetPeriod(periodUsec);
    //s.DebugSetDutyCycle(50);

    SignalDAC s(&sineWave);
    //s.SetPeriod(833);
    s.SetPeriod(454);
    s.Start();

    #if 0
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
    #endif

    evm.MainLoop();
}

void loop() {}









