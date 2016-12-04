#include "Evm.h"
#include "StepperController.h"
#include "StepperControllerSerialInterface.h"


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

StepperControllerSerialIface<StepperControllerAsync<StepperControllerBipolar>> scsi(sca, Serial);

void setup()
{
    Serial.begin(9600);

    scsi.Init();

    evm.MainLoop();
}

void loop() {}
















