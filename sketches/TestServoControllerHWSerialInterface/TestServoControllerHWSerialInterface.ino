#include "Evm.h"
#include "ServoControllerHW.h"
#include "ServoControllerSerialInterface.h"


static Evm::Instance<10,10,10> evm;

static ServoControllerHWOwner<Timer1>  sco;
//static ServoControllerHWOwner<Timer2>  sco;
static ServoControllerHW              *sc1A = sco.GetServoControllerHWA();
static ServoControllerSerialIface<ServoControllerHW> scsi(*sc1A, Serial);


void setup()
{
    Serial.begin(9600);

    sco.Init();
    scsi.Init();

    evm.MainLoop();
}

void loop() {}



