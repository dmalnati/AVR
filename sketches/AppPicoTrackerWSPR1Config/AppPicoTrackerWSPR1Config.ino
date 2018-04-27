#include "AppPicoTrackerWSPR1.h"


static AppPicoTrackerWSPR1Config cfg =
{
    // GPS
    .pinGpsEnable   = 26,
    .pinGpsSerialRx = 23,
    .pinGpsSerialTx = 24,

    // WSPR TX
    .pinWsprTxEnable = 0,

    // Status LEDs
    .pinLedRed   = 13,
    .pinLedGreen = 12,
};

static AppPicoTrackerWSPR1 app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


