#include "AppPicoTrackerWSPR2.h"
//#include "AppPicoTrackerWSPR2Test.h"


static AppPicoTrackerWSPR2Config cfg =
{
    // Human interfacing
    .pinConfigure = 16, // pin A
    
    // GPS
    .pinGpsBackupPower =  5,
    .pinGpsEnable      = 14,
    .pinGpsSerialRx    = 23,
    .pinGpsSerialTx    = 24,

    .gpsMaxDurationLocationLockWaitMs = 150000UL, // 2m30 - 85th percentile cold start success
    .gpsMaxDurationTimeLockWaitMs     =  10000UL,  // 10 seconds
    .gpsDurationWaitPostResetMs       =  30000UL,  // 30 seconds

    // WSPR TX
    .pinWsprTxEnable = 15,

    // Status LEDs
    .pinLedRed   = 12,
    .pinLedGreen = 13,
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
