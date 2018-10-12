#include "Log.h"
#include "LogBlob.h"
#include "Evm.h"
#include "SensorIR_Remote.h"


static Evm::Instance<10,10,10> evm;

static const uint8_t PIN_IR_1 = 15;
static SensorIR_Remote ir(PIN_IR_1);


void setup()
{
    LogStart(9600);
    Log("Starting");

    ir.SetCallback([](SensorIR_Remote::Button b){
        uint32_t bitBuffer = (uint32_t)b;
        
        Log("Val: ", bitBuffer, "; ", LogBIN(bitBuffer));
    });
    ir.Start();

    Log("Running");
    evm.MainLoop();
}

void loop() {}



