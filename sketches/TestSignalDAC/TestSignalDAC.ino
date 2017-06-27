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
        TestSignalDACForever();
    });
    ted.RegisterForTimedEventInterval(1000);

    
    evm.MainLoop();
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


















