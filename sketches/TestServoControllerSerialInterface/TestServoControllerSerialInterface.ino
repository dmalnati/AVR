#include "Evm.h"
#include "ServoController.h"
#include "ServoControllerSerialInterface.h"


static Evm::Instance<10,10,10> evm;

static uint8_t pinServo = 9;

static ServoController sc(pinServo);
static ServoControllerSerialIface<ServoController> scsi(sc, Serial);

void setup()
{
    Serial.begin(9600);

    scsi.Init();

    evm.MainLoop();
}

void loop() {}
















