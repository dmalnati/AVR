#include "AppPicoTrackerWSPR2.h"
#include "AppPicoTrackerWSPR2Test.h"


static AppPicoTrackerWSPR2Config cfg =
{
    // Human interfacing
    .pinConfigure = 26, // pin A
    
    // GPS
    .pinGpsBackupPower = 11,
    .pinGpsEnable      =  5,
    .pinGpsSerialRx    = 13,
    .pinGpsSerialTx    = 12,

    .gpsMaxDurationLocationLockWaitMs = 300000UL,  // 5 minutes, 2x the 85-percentile cold start success
    .gpsMaxDurationTimeLockWaitMs     =  10000UL,  // 10 seconds
    .gpsDurationWaitPostResetMs       =  30000UL,  // 30 seconds

    // WSPR TX
    .pinWsprTxEnable = 14,

    // Status LEDs
    .pinLedRed   = 24,
    .pinLedGreen = 23,
};

#if 1
static AppPicoTrackerWSPR2 app(cfg);
#else
static AppPicoTrackerWSPR2Test app(cfg);
#endif

void setup()
{
    app.Run();
}

void loop() {}
