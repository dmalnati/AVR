#include "Evm.h"
#include "ServoController.h"
#include "ServoControllerHW.h"
#include "PinInputAnalog.h"


static const uint8_t PIN_ANALOG_X = 27;
static const uint8_t PIN_ANALOG_Y = 28;

static Evm::Instance<20,20,20> evm;

static PinInputAnalog  piaX(PIN_ANALOG_X);
static PinInputAnalog  piaY(PIN_ANALOG_Y);
static ServoControllerHWOwner1 sco1;
static ServoControllerHW &scX = sco1.GetServoControllerHWA(); // pin 15
static ServoControllerHW &scY = sco1.GetServoControllerHWB(); // pin 16
//static ServoController scX(15);
//static ServoController scY(16);


static void MoveX(uint16_t val)
{
    uint8_t deg = (double)val * 180.0 / 1023.0;

    Serial.print("MoveX to "); Serial.println(deg);
    scX.MoveTo(deg);
}

static void MoveY(uint16_t val)
{
    uint8_t deg = (double)((1023 - val) + 512) * 180.0 / 1023.0;

    Serial.print("MoveY to "); Serial.println(deg);
    scY.MoveTo(deg);
    
    if (0 && val >= 450)
    {
        //uint8_t deg = (double)((1023 - val) + 450) * 180.0 / 1023.0;
        uint8_t deg = (double)((1023 - val) + 450) * 180.0 / 1023.0;
        //uint8_t deg = (double)val * 180.0 / 1023.0;

        Serial.print("MoveY to "); Serial.println(deg);
        scY.MoveTo(deg);
    }
}


void setup()
{
    Serial.begin(9600);

    sco1.Init();

    Serial.println("before piaX");
    piaX.SetCallback([](uint16_t val) { MoveX(val); });
    piaX.Enable();


    Serial.println("before piaY");
    piaY.SetCallback([](uint16_t val) { MoveY(val); });
    piaY.Enable();

    scY.SetModeInverted();
    

    Serial.println("before MainLoop");
    evm.MainLoop();
}

void loop() {}










