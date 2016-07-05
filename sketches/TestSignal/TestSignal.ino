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


//static SignalSourceSineWave<SINE_WAVE_STEP_COUNT> sineWave;
//static SignalDAC s(&sineWave);

void setup()
{
    Serial.begin(9600);

    PAL.PinMode(pinSignalA, OUTPUT);
    PAL.PinMode(pinSignalB, OUTPUT);


    // All relates to speed of timer, actually...
    // Max speed is 31,250 Hz
    // That's 0.125us per tick
    // That's 32us per loop
    // 1200 Hz signal needs one sine wave per 1000 / 1200 = 833usec
    // So 833us / 32 = 26 iterations.
    // 360 degrees / 26 iterations = 360 / 26 = 13.8 degrees per iteration
    uint8_t phaseStep1200 = 13;

    // 2200 Hz signal needs one sine wave per 1000 / 2200 = 454usec
    // So 454us / 32 = 14 iterations = 360 / 14 = 25.7 degrees per iteration
    uint8_t phaseStep2200 = 25;

    // Baud = 1200Hz
    // So that's the same count of iterations as the 1200Hz signal
    // So 1 bit time == 26 iterations

    uint8_t bitList[] = { 0, 1, 1, 0, 1, 0, 0, 0, 1 };
    uint8_t bitListLen = sizeof(bitList);
    int8_t  bitValLast = -1;

    SignalSourceSineWave ss;
    ss.Reset(bitList[0] ? phaseStep1200 : phaseStep2200);

    uint32_t count = 1;
    uint8_t  bitSeparator = 1;
    for (uint8_t i = 0; i < bitListLen; ++i)
    {
        uint8_t bitVal = bitList[i];

        if (bitVal != bitValLast)
        {
            ss.ChangePhaseStep(bitVal ? phaseStep1200 : phaseStep2200);
        }

        for (uint8_t j = 0; j < 26; ++j)
        {
            uint8_t sample = ss.GetSample();
            ss.GetNextSampleReady();

            // columns: count, bitSeparator, 0-val, 1-val
            Serial.print(count);
            Serial.print(",");
            Serial.print(bitSeparator * 255);
            Serial.print(",");
            Serial.print(!bitVal ? sample : 0);
            Serial.print(",");
            Serial.print(bitVal ? sample : 0);
            Serial.println();

            ++count;
        }
        bitSeparator = !bitSeparator;
        
        bitValLast = bitVal;
    }



    #if 0
    Serial.println("Sine Wave Samples:");
    for (uint16_t i = 0; i < 100; ++i)
    {
        Serial.println(ss.GetSample());
        ss.GetNextSampleReady();
    }
    Serial.println();
    #endif
    

    //uint16_t periodUsec = 10000;
    //s.SetPeriod(periodUsec);
    //s.DebugSetDutyCycle(50);

    //SignalDAC s(&sineWave);
    //s.SetPeriod(833);
    //s.SetPeriod(454);
    //s.Start();


    evm.MainLoop();
}

void loop() {}









