#include "Evm.h"
#include "ServoController.h"


static const uint8_t PIN_SERVO = 25;

static Evm::Instance<10,10,10> evm;
static ServoController         sc(PIN_SERVO);


void setup()
{
    sc.MoveTo(135); // implies 1750us pulses

    evm.MainLoop();
}

void loop()
{
}
