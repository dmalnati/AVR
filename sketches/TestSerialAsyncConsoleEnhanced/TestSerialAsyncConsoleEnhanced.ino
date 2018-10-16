#include "Evm.h"
#include "Log.h"
#include "SerialInput.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<10> shell;


void setup()
{
    LogStart(9600);
    Log("Starting");

    shell.Start();

    evm.MainLoop();
}

void loop() {}







