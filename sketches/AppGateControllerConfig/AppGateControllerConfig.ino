#include "AppGateController.h"


static AppGateControllerConfig cfg =
{
    .myGateCombo = 0b0000001110000111,
    .myGateFreq  = 310000000,
    
    .pinSlaveSelect = 23,
    .pinShutdown    = 14,
    .pinOOK         = 5,

    .pinLed1 = 10,
    .pinLed2 = 11,
    .pinLed3 = 12,
    .pinLed4 = 13,

    .pinButtonStartBruteForce = 28,
    .pinButtonStartForceOpen  = 27,
    .pinButtonStartJam        = 26,
    .pinButtonStop            = 25,
};

static AppGateController app(cfg);


void setup()
{
    app.Run();
}

void loop() {}
