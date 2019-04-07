#include "AppPicoTrackerWSPR1.h"


static AppPicoTrackerWSPR1Config cfg =
{
    // Human interfacing
    .pinConfigure = 25,
    
    // Pre-regulator power sensing
    .pinInputVoltage = 26,
    
    // Regulator control
    .pinRegPowerSaveEnable = 6,
    
    // GPS
    .pinGpsEnable   = 14,
    .pinGpsSerialRx = 23,
    .pinGpsSerialTx = 24,

    // WSPR TX
    .pinWsprTxEnable = 15,

    // Status LEDs
    .pinLedRed   = 12,
    .pinLedGreen = 13,
};

static AppPicoTrackerWSPR1 app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


