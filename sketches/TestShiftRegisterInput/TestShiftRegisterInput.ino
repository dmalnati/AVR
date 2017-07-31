#include "Evm.h"
#include "ShiftRegister.h"
#include "ShiftRegisterInput.h"


static const uint8_t PIN_LOAD         = 14;
static const uint8_t PIN_CLOCK        = 13;
static const uint8_t PIN_CLOCK_ENABLE = 12;
static const uint8_t PIN_SERIAL       = 11;


static Evm::Instance<10,10,10> evm;

static ShiftRegister sr(PIN_LOAD, PIN_CLOCK, PIN_CLOCK_ENABLE, PIN_SERIAL);
static ShiftRegisterInput<2> sri(sr, 20, [](uint8_t logicLevel){
    Serial.print("default logicLevel change: ");
    Serial.print(logicLevel);
    Serial.println();
});


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

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
        Serial.print("Setting up pin ");
        Serial.print(i);
        Serial.print(", mode: ");
        Serial.print((i % 3) == 0 ? "LEVEL_RISING" : (i % 3) == 1 ? "LEVEL_FALLING" : "LEVEL_RISING_AND_FALLING");
        Serial.println();
        
        sri.SetCallback(i, [=](uint8_t logicLevel){
            Serial.print("Pin ");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(logicLevel);
            Serial.println();
        }, modeList[i % 3] );
    }
}


void loop() {}


















