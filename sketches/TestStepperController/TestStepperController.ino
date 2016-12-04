#include "PAL.h"
#include "Evm.h"
#include "StepperController.h"


static Evm::Instance<10,10,10> evm;

static uint8_t pinEnable   = 10;
static uint8_t pinPhase1S1 = 11;
static uint8_t pinPhase1S2 = 12;
static uint8_t pinPhase2S1 = 13;
static uint8_t pinPhase2S2 = 14;

static StepperControllerBipolar sc(pinEnable,
                                   pinPhase1S1,
                                   pinPhase1S2,
                                   pinPhase2S1,
                                   pinPhase2S2);



void GoRightForAWhile(uint16_t steps, uint16_t delayMs)
{
    while (steps)
    {
        sc.HalfStepRight();

        PAL.Delay(delayMs);
        
        --steps;
    }
}

void GoLeftForAWhile(uint16_t steps, uint16_t delayMs)
{
    while (steps)
    {
        sc.HalfStepLeft();

        PAL.Delay(delayMs);
        
        --steps;
    }
}

void setup()
{
    uint16_t steps   = 750;
    uint16_t delayMs = 1;

    Pin pinDebug(15, LOW);
    
    while (1)
    {
        PAL.DigitalWrite(pinDebug, HIGH);
        
        GoRightForAWhile(steps, delayMs);
        GoLeftForAWhile(steps, delayMs);
        
        PAL.DigitalWrite(pinDebug, LOW);
    }
}

void loop() {}


