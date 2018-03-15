#include "AppPicoTracker1.h"


static AppPicoTracker1Config cfg =
{
    // GPS
    .pinGpsEnable   = 26,
    .pinGpsSerialRx = 23,
    .pinGpsSerialTx = 24,

    // Status LEDs
    .pinLedRed   = 13,
    .pinLedGreen = 12,
    .pinLedBlue  = 11,

    // Radio
    .pinRadioSlaveSelect = 25,
    .pinRadioShutdown    = 14,
};

static AppPicoTracker1 app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


