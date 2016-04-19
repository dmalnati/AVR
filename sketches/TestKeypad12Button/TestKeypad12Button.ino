#include "Evm.h"
#include "Keypad12Button.h"


static Evm::Instance<10,10,10> evm;

static Keypad12Button kpad({
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19
});


void setup()
{
    kpad.Init([](char) {
        // Nothing to do
    });

    evm.MainLoop();
}

void loop() {}






