#include "AppHABTracker1.h"


static AppHABTracker1Config cfg =
{
    // Application configuration
    .reportIntervalMs = 1000,
    
    // SD Logger
    .pinSerialTxSdLogger = 25,

    // GPS
    .pinSerialRxGPS = 23,
    .pinSerialTxGPS = 24,

    // APRS
    .dstCallsign      = "APZ001",
    .dstSsid          = 0,
    .srcCallsign      = "KD2KDD",
    .srcSsid          = 9,
    .repeaterCallsign = "WIDE1",
    .repeaterSsid     = 1,
};

static AppHABTracker1 app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


