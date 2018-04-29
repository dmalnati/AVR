#include "Evm.h"
#include "ShiftRegisterIn.h"
#include "ShiftRegisterInputSingleCallback.h"


static const uint8_t PIN_LOAD         =  9;
static const uint8_t PIN_CLOCK        = 10;
static const uint8_t PIN_CLOCK_ENABLE = 14;
static const uint8_t PIN_SERIAL       = 15;


static Evm::Instance<10,10,10> evm;

static ShiftRegisterIn sr(PIN_LOAD, PIN_CLOCK, PIN_CLOCK_ENABLE, PIN_SERIAL);
static ShiftRegisterInputSingleCallback<3> sri(sr);


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    sri.SetCallback([](uint8_t pinLogical, uint8_t logicLevel){
        @fix@Serial.print("Pin ");
        @fix@Serial.print(pinLogical);
        @fix@Serial.print(" changed from ");
        @fix@Serial.print(!logicLevel);
        @fix@Serial.print(" to ");
        @fix@Serial.print(logicLevel);
        @fix@Serial.println();
    });

    uint8_t forceRefresh = 1;
    sri.Init(forceRefresh);

    evm.MainLoop();
}

void loop() {}


















