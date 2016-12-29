#include "Evm.h"
#include "ServoController.h"
#include "PinInputAnalog.h"



static const uint8_t PIN_ANALOG_X = 27;
static const uint8_t PIN_ANALOG_Y = 28;
static const uint8_t PIN_SERVO_X  = 10;
static const uint8_t PIN_SERVO_Y  = 11;

static Evm::Instance<10,10,10> evm;

static PinInputAnalog  piaX(PIN_ANALOG_X);
static PinInputAnalog  piaY(PIN_ANALOG_Y);
static ServoController scX(PIN_SERVO_X);
static ServoController scY(PIN_SERVO_Y);


static void MoveX(uint16_t val)
{
    uint8_t deg = (double)val * 180.0 / 1023.0;

    scX.MoveTo(deg);
}

static void MoveY(uint16_t val)
{
    uint8_t deg = (double)val * 180.0 / 1023.0;

    scY.MoveTo(deg);
}


void setup()
{
    Serial.begin(9600);

    piaX.SetCallback([](uint16_t val) { MoveX(val); });
    piaX.Enable();

    piaY.SetCallback([](uint16_t val) { MoveY(val); });
    piaY.Enable();

    evm.MainLoop();
}

void loop() {}










