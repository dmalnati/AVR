#include "Evm.h"
#include "ServoControllerHW.h"


static Evm::Instance<10,10,10> evm;

static ServoControllerHWOwner<Timer1>  sco1;
static ServoControllerHWOwner<Timer2>  sco2;
static ServoControllerHW              &sc1A = sco1.GetServoControllerHWA(); // pin 15
static ServoControllerHW              &sc1B = sco1.GetServoControllerHWB(); // pin 16
static ServoControllerHW              &sc2A = sco2.GetServoControllerHWA(); // pin 17
static ServoControllerHW              &sc2B = sco2.GetServoControllerHWB(); // pin  5


void setup()
{
    @fix@Serial.begin(9600);

    sco1.Init();
    sco2.Init();

    sc1A.MoveTo(45);
    sc1B.MoveTo(135);
    sc2A.MoveTo(45);
    sc2B.MoveTo(135);

    evm.MainLoop();
}

void loop() {}



