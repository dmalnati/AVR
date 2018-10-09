#include "PAL.h"
#include "Log.h"
#include "Evm.h"
#include "PinInputAnalog.h"


static uint8_t PIN_ANALOG_1 = 28;
static uint8_t PIN_ANALOG_2 = 27;

static Evm::Instance<10,10,10> evm;
static PinInputAnalog pia1(PIN_ANALOG_1);
static PinInputAnalog pia2(PIN_ANALOG_2);


static void OnAnalogRead(uint8_t pin, uint16_t val)
{
    Log(PAL.Millis());
    
    Log(pin, ": ", val);
    LogNL();
}

void setup()
{
    LogStart(9600);
    Log("Starting");

    pia1.SetCallback([=](uint16_t val) { OnAnalogRead(PIN_ANALOG_1, val); });
    pia1.Enable();
    pia2.SetCallback([=](uint16_t val) { OnAnalogRead(PIN_ANALOG_2, val); });
    pia2.Enable();

    Log("Running");
    evm.MainLoop();
}

void loop() {}



