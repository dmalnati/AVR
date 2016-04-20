#include "Evm.h"
#include "Utl.h"


static Evm::Instance<10,10,10> evm;

static TimedPinToggler      ttoggler1(26);
static TimedPinToggler      ttoggler2(25);
static IdlePinToggler       itoggler(24);
static TimedPinTogglerHiRes ittoggler(23);


void setup()
{
    ttoggler1.RegisterForTimedEventInterval(1); // ~2ms interval
    ttoggler2.RegisterForTimedEventInterval(20);
    itoggler.RegisterForIdleTimeEvent();
    ittoggler.RegisterForIdleTimeHiResTimedEventInterval(1234);

    evm.MainLoop();
}

void loop() {}



