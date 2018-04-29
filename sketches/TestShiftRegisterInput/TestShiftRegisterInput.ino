#include "Evm.h"
#include "ShiftRegisterIn.h"
#include "ShiftRegisterInput.h"


static const uint8_t PIN_LOAD         = 14;
static const uint8_t PIN_CLOCK        = 13;
static const uint8_t PIN_CLOCK_ENABLE = 12;
static const uint8_t PIN_SERIAL       = 11;


static Evm::Instance<10,10,10> evm;

static ShiftRegisterIn sr(PIN_LOAD, PIN_CLOCK, PIN_CLOCK_ENABLE, PIN_SERIAL);
static ShiftRegisterInput<2> sri(sr, 20, [](uint8_t logicLevel){
    @fix@Serial.print("default logicLevel change: ");
    @fix@Serial.print(logicLevel);
    @fix@Serial.println();
});


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    Monitor(2);

    uint8_t forceRefresh = 1;
    sri.Init(forceRefresh);

    evm.MainLoop();
}

void Monitor(uint8_t srCount)
{
    uint8_t modeList[] = { LEVEL_RISING, LEVEL_FALLING, LEVEL_RISING_AND_FALLING };
    
    for (uint8_t i = 0; i < (srCount * 8) - 1; ++i)
    {
        @fix@Serial.print("Setting up pin ");
        @fix@Serial.print(i);
        @fix@Serial.print(", mode: ");
        @fix@Serial.print((i % 3) == 0 ? "LEVEL_RISING" : (i % 3) == 1 ? "LEVEL_FALLING" : "LEVEL_RISING_AND_FALLING");
        @fix@Serial.println();
        
        sri.SetCallback(i, [=](uint8_t logicLevel){
            @fix@Serial.print("Pin ");
            @fix@Serial.print(i);
            @fix@Serial.print(": ");
            @fix@Serial.print(logicLevel);
            @fix@Serial.println();
        }, modeList[i % 3] );
    }
}


void loop() {}


















