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

static StepperControllerAsync<StepperControllerBipolar> sca(sc);


void GoRightForAWhile(uint16_t steps, uint16_t delayMs)
{
    while (steps)
    {
        sc.HalfStepCW();

        PAL.Delay(delayMs);
        
        --steps;
    }
}

void GoLeftForAWhile(uint16_t steps, uint16_t delayMs)
{
    while (steps)
    {
        sc.HalfStepCCW();

        PAL.Delay(delayMs);
        
        --steps;
    }
}

void setup()
{
    uint16_t steps   = 600;
    uint16_t delayMs = 3;

    Pin pinDebug(15, LOW);
    
    while (1)
    {
        // Manually drive the stepper for a while
        PAL.DigitalWrite(pinDebug, HIGH);
        
        GoRightForAWhile(steps, delayMs);
        GoLeftForAWhile(steps, delayMs);

        
        
        // Async drive with step limits
        PAL.DigitalWrite(pinDebug, LOW);

        sca.HalfStepCW(steps, delayMs, [](){
            Evm::GetInstance().EndMainLoop();
        });

        evm.HoldStackDangerously();

        sca.HalfStepCCW(steps, delayMs, [](){
            Evm::GetInstance().EndMainLoop();
        });

        evm.HoldStackDangerously();


        // Async drive forever but stop after time
        PAL.DigitalWrite(pinDebug, HIGH);

        sca.HalfStepForeverCW(delayMs);
        evm.HoldStackDangerously(3000);
        sca.Stop();

        sca.HalfStepForeverCCW(delayMs);
        evm.HoldStackDangerously(3000);
        sca.Stop();


        PAL.DigitalWrite(pinDebug, LOW);
        PAL.Delay(50);
    }
}

void loop() {}


