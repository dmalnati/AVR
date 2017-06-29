#include "PAL.h"
#include "Evm.h"

#include "MidiSynthesizer.h"


static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;

static MidiCommandFromSerial midiCmdFromSerial;
static MidiCommandParser midiCmdParser;
static MidiSynthesizer midiSynth;





void TestMidiCore()
{
    midiSynth.Start();

    while (1)
    {
        MidiCommand m;
        
        if (midiCmdFromSerial.GetMidiCommand(&m))
        {
            midiSynth.ProcessCommand(m);
        }
    }

    midiSynth.Stop();
}


void setup()
{
    Serial.begin(9600);

    midiCmdFromSerial.Init(&midiCmdParser);
    midiSynth.Init();

    ted.SetCallback([](){
        Serial.println("Looping again");

        TestMidiCore();
    });
    ted.RegisterForTimedEventInterval(1000);
    
    evm.MainLoop();
}




void loop() {}


















