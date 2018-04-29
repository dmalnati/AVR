#include "Evm.h"
#include "StepperController.h"
#include "StepperControllerSerialInterface.h"


static Evm::Instance<10,10,10> evm;

static uint8_t pinEnable   = 0;
static uint8_t pinPhase1S1 = 26;
static uint8_t pinPhase1S2 = 25;
static uint8_t pinPhase2S1 = 24;
static uint8_t pinPhase2S2 = 23;

static StepperControllerUnipolar sc(pinEnable,
                                    pinPhase1S1,
                                    pinPhase1S2,
                                    pinPhase2S1,
                                    pinPhase2S2);

static StepperControllerAsync<StepperControllerUnipolar> sca(sc);

StepperControllerSerialIface<StepperControllerAsync<StepperControllerUnipolar>> scsi(sca, Serial);

void setup()
{
    @fix@Serial.begin(9600);

    scsi.Init();

    evm.MainLoop();
}

void loop() {}
















