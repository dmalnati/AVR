#include "Evm.h"
#include "SensorEcompassLSM303C.h"



static const uint32_t DEBUG_INTERVAL_MS = 1000;

static Evm::Instance<10,10,10> evm;

static TimedEventHandlerDelegate ted;
static SensorEcompassLSM303C sensor();


void setup()
{
    ted.SetCallback([&]() {

    });
    
    ted.RegisterForTimedEventInterval(DEBUG_INTERVAL_MS);
    
    evm.MainLoop();
}

void loop() {}


