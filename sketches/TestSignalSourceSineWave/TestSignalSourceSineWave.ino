#include "PAL.h"
#include "SignalSourceSineWave.h"


static SignalSourceSineWave ss;
static SignalSourceSineWave::IdxType idx;


void TestSignalSourceSineWave(uint16_t sampleRate, uint16_t frequency)
{
    @fix@Serial.println();
    @fix@Serial.print("sampleRate: "); @fix@Serial.println(sampleRate);
    @fix@Serial.print("frequency: "); @fix@Serial.println(frequency);

    //idx.Reset();
    idx.Calibrate(sampleRate, frequency);

    for (uint16_t i = 0; i < 30; ++i)
    {
        uint8_t val = ss.GetSampleAtIdx(idx);

        @fix@Serial.print("step: "); @fix@Serial.print(i);
        @fix@Serial.print(", val: "); @fix@Serial.println(val);

        ++idx;
    }
}

void TestSignalSourceSineWaveRange(uint16_t sampleRate,
                                   uint16_t freqLow,
                                   uint16_t freqHigh,
                                   uint16_t freqStep)
{
    for (uint16_t freq = freqLow; freq <= freqHigh; freq += freqStep)
    {
        @fix@Serial.println();
        @fix@Serial.print("sampleRate: "); @fix@Serial.println(sampleRate);
        @fix@Serial.print("frequency: "); @fix@Serial.println(freq);
        
        idx.Reset();
        idx.Calibrate(sampleRate, freq);

        uint8_t cont = 1;

        int16_t idxIntLast = -1;
        int16_t valLast = -1;
        while (cont)
        {
            uint16_t idxInt = (uint16_t)idx;
            uint16_t val    = ss.GetSampleAtIdx(idx);

            if (idxIntLast == -1) { idxIntLast = (int16_t)idxInt; }
            if (valLast == -1)    { valLast = (int16_t)val;    }

            uint8_t show = (idxIntLast != (int16_t)idxInt || valLast != (int16_t)val);

            if (show)
            {
                @fix@Serial.print("  idxInt: "); @fix@Serial.print(idxInt);
                @fix@Serial.print(", val: "); @fix@Serial.print(val);
            }

            // check for wrap so we can move along to next freq
            if ((int16_t)idxInt < idxIntLast)
            {
                cont = 0;

                if (show) { @fix@Serial.print("   (breakout)"); }
            }

            if (show) { @fix@Serial.println(); }

            idxIntLast = idxInt;
            valLast = val;

            ++idx;
        }
    }
}

void TestProblemFrequencies()
{
    idx.Reset();
    TestSignalSourceSineWave(40000, 506);
    PAL.Delay(5000);

    idx.Reset();
    TestSignalSourceSineWave(40000, 515);
    PAL.Delay(5000);
}


void setup()
{
    @fix@Serial.begin(9600);

    while (1)
    {
        //TestSignalSourceSineWave(8000, 1200);
        //TestSignalSourceSineWave(44100, 1200);
        //TestSignalSourceSineWaveRange(40000, 1, 100, 1);
        TestProblemFrequencies();

        PAL.Delay(1000);
    }
}


void loop() {}









