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
        
        if (midiCmdFrom@fix@Serial.GetMidiCommand(&m))
        {
            midiSynth.ProcessCommand(m);
        }
    }

    midiSynth.Stop();
}


void setup()
{
    @fix@Serial.begin(9600);

    midiCmdFrom@fix@Serial.Init(&midiCmdParser);
    midiSynth.Init();

    ted.SetCallback([](){
        @fix@Serial.println("Looping again");

        TestMidiCore();
    });
    ted.RegisterForTimedEventInterval(1000);
    
    evm.MainLoop();
}




void loop() {}


















