#include "Evm.h"
#include "Log.h"
#include "RotaryEncoderWithButtonInput.h"


static const uint8_t PIN_A = 15;
static const uint8_t PIN_B = 16;
static const uint8_t PIN_BUTTON = 17;

static Evm::Instance<10,10,10> evm;
static RotaryEncoderWithButtonInput input(PIN_A, PIN_B, PIN_BUTTON);


void setup()
{
    LogStart(9600);
    Log("Starting");

    uint8_t count = 0;
    input.SetCallbackOnRotate([&count](int8_t leftOrRight){
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
    input.SetCallbackOnPressChange([&count](uint8_t logicLevel){
        ++count;
        
        Log("Press(", logicLevel, ") ", count);
    });
    input.Enable();

    evm.MainLoop();
}

void loop() {}







