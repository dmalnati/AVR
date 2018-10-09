#include "PAL.h"
#include "Log.h"
#include "Evm.h"
#include "PinInput.h"


static uint8_t PIN_ANALOG_1 = 28;
static uint8_t PIN_ANALOG_2 = 27;

static Evm::Instance<10,10,10> evm;
static PinInput pi1(PIN_ANALOG_1);
static PinInput pi2(PIN_ANALOG_2);


static void OnRead(uint8_t pin, uint16_t val)
{
    Log(PAL.Millis());
    
    Log(pin, ": ", val);
    LogNL();
}

void setup()
{
    LogStart(9600);
    Log("Starting");

    pi1.SetCallback([=](uint16_t val) { OnRead(PIN_ANALOG_1, val); });
    pi1.Enable();
    pi2.SetCallback([=](uint16_t val) { OnRead(PIN_ANALOG_2, val); });
    pi2.Enable();

    Log("Running");
    evm.MainLoop();
}

void loop() {}



