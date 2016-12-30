#include "Evm.h"
#include "ServoControllerHW.h"
#include "ServoControllerSerialInterface.h"


static Evm::Instance<10,10,10> evm;

static ServoControllerHWOwner1  sco1;
//static ServoControllerHWOwner2  sco2;
static ServoControllerHW              &sc1A = sco1.GetServoControllerHWA();
static ServoControllerSerialIface<ServoControllerHW> scsi(sc1A, Serial);


void setup()
{
    Serial.begin(9600);

    sco1.Init();
    scsi.Init();

    evm.MainLoop();
}

void loop() {}



