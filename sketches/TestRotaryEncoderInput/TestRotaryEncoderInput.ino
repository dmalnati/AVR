#include "Evm.h"
#include "Log.h"
#include "RotaryEncoderInput.h"


static const uint8_t PIN_A = 15;
static const uint8_t PIN_B = 16;

static Evm::Instance<10,10,10> evm;
static RotaryEncoderInput rei(PIN_A, PIN_B);


void setup()
{
    LogStart(9600);
    Log("Starting");

    uint8_t count = 0;
    rei.SetCallback([&count](int8_t leftOrRight){
        ++count;
        
        if (leftOrRight == -1)
        {
            Log("Left ", count);
        }
        else
        {
            Log("Right ", count);
        }
    });
    rei.Enable();

    evm.MainLoop();
}

void loop() {}







