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

    // Transmitter
    .flagStartDurationMs     = 300,
    .flagEndDurationMs       = 10,
    .transmitCount           = 2,
    .delayMsBetweenTransmits = 2000,
};

static AppHABTracker1 app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


