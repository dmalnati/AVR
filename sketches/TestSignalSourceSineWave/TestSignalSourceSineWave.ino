#include "PAL.h"
#include "SignalSourceSineWave.h"


static SignalSourceSineWave ss;
static SignalSourceSineWave::IdxType idx;


void setup()
{
    Serial.begin(9600);

    while (1)
    {
        TestSignalSourceSineWave(8000, 1200);
        TestSignalSourceSineWave(44100, 1200);

        PAL.Delay(1000);
    }
}

void TestSignalSourceSineWave(uint16_t sampleRate, uint16_t frequency)
{
    Serial.println();
    Serial.print("sampleRate: "); Serial.println(sampleRate);
    Serial.print("frequency: "); Serial.println(frequency);

    //idx.ResetIdx();
    idx.Calibrate(sampleRate, frequency);

    for (uint16_t i = 0; i < 30; ++i)
    {
        uint8_t val = ss.GetSampleAtIdx(idx);

        Serial.print("step: "); Serial.print(i);
        Serial.print(", val: "); Serial.println(val);

        ++idx;
    }
}


void loop() {}









