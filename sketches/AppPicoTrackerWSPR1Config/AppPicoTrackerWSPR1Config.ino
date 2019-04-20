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
    // I choose 2 hours old as a cutoff, despite having seen much longer
    // durations go by with accurate time on starup.
    .gpsMaxAgeLocationLockMs = 2UL * 60UL * 60UL * 1000UL,
    // I choose 5 seconds as a cutoff as I have seen in practice this
    // works well.
    .gpsMaxDurationTimeLockWaitMs = 10000,

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


