#include "AppGateController.h"


static AppGateControllerConfig cfg =
{
    .myGateCombo = 0b0000001110000111,
    .myGateFreq  = 310000000,
    
    .pinSlaveSelect = 25,
    .pinShutdown    = 14,
    .pinOOK         = 5,

    .pinLed1 = 10,
    .pinLed2 = 11,
    .pinLed3 = 12,
    .pinLed4 = 13,
};

static AppGateController app(cfg);


void setup()
{
    app.Run();
}

void loop() {}
