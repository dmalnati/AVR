#define IVM_DISABLE

#include "Evm.h"
#include "SensorGPSUblox.h"



static const int8_t PIN_RX = 0;
static const int8_t PIN_TX = 0;


static Evm::Instance<10,10,10> evm;
static SensorGPSUblox gps(PIN_RX, PIN_TX);


void setup()
{
    
    evm.MainLoop();
}

void loop() {}


