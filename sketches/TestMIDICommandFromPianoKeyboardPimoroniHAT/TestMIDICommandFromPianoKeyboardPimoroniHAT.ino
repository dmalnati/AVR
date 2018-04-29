#include "Evm.h"
#include "MIDICommandFromPianoKeyboardPimoroniHAT.h"


static Evm::Instance<10,10,10> evm;
static MIDICommandFromPianoKeyboardPimoroniHAT hatToMidi;


void setup()
{
    @fix@Serial.begin(31250);

    hatToMidi.GetHat().EnableLEDs();
    hatToMidi.Init();

    evm.MainLoop();
}


void loop() {}


















