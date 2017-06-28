#include "PAL.h"
#include "Evm.h"

#include "SignalSourceSineWave.h"
#include "SignalDAC.h"


static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;

static SignalDAC<SignalSourceSineWave> dac;


void setup()
{
    Serial.begin(9600);

    
    ted.SetCallback([](){
        Serial.println("Looping again");
        
        //TestSignalDAC();
        //TestSignalDACForever();
        //TestSignalDACDoingHigherSampleRates();
        TestSignalDACRamp();
    });
    ted.RegisterForTimedEventInterval(1000);

    
    evm.MainLoop();
}

void TestSignalDACDoingHigherSampleRates()
{
    uint16_t frequency = 440;

    TestSignalDACAt(1000, frequency);
    TestSignalDACAt(2000, frequency);
    TestSignalDACAt(4000, frequency);
    TestSignalDACAt(8000, frequency);
    TestSignalDACAt(16000, frequency);
    TestSignalDACAt(24000, frequency);
    TestSignalDACAt(32000, frequency);
    TestSignalDACAt(40000, frequency);
}



void TestSignalDACRampFrequency(uint16_t sampleRate,
                                uint16_t freqStart,
                                uint16_t freqEnd,
                                uint16_t freqStep)
{
    dac.SetSampleRate(sampleRate);

    uint16_t frequency = freqStart;

    // need to set the frequency before starting or
    // the prior frequency will still be in effect.
    dac.SetFrequency(frequency);

    dac.Start();
    do
    {
        dac.SetFrequency(frequency);
        
        PAL.Delay(500);
        
        frequency += freqStep;
    } while (frequency <= freqEnd);
    dac.Stop();
}

void TestSignalDACRamp()
{
    TestSignalDACRampFrequency(36000, 1, 10, 1);
    TestSignalDACRampFrequency(36000, 440, 4400, 440);
}

void TestSignalDAC()
{
    TestSignalDACAt(8000, 440);
    TestSignalDACAt(8000, 1200);
    TestSignalDACAt(16000, 1200);
    TestSignalDACAt(40000, 1200);
    TestSignalDACAt(44100, 1200);
}

void TestSignalDACAt(uint16_t sampleRate, uint16_t frequency)
{
    dac.SetSampleRate(sampleRate);
    dac.SetFrequency(frequency);

    dac.Start();

    PAL.Delay(100);

    dac.Stop();
}

void TestSignalDACForever()
{
    TestSignalDACForeverAt(16000);
}

void TestSignalDACForeverAt(uint16_t sampleRate)
{
    dac.SetSampleRate(sampleRate);

    dac.Start();

    while (1)
    {
        dac.SetFrequency(440);
        PAL.Delay(5000);

        dac.SetFrequency(880);
        PAL.Delay(5000);

        dac.SetFrequency(1660);
        PAL.Delay(5000);

        dac.SetFrequency(2200);
        PAL.Delay(5000);
    }
    
    

    dac.Start();

    while (1) {}
}


void loop() {}


















