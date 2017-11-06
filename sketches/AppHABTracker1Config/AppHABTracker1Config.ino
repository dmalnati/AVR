#include "AppHABTracker1.h"


static AppHABTracker1Config cfg =
{
    // Application configuration
    .pinSerialDebugOutput = 26,
    
    .aprsReportIntervalMs = 15000,
    .logIntervalMs        =  1000,

    .pinLedHeartbeat     = 18,
    .heartbeatIntervalMs = 500,

    .pinLedGpsLock         = 19,
    .gpsLockGoodAgeLimitMs = 30000,

    .pinLedTransmitting = 15,

    
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
    .pinTxEnable = 14,
    
    .radioWarmupDurationMs = 20,
    
    .flagStartDurationMs     = 500,
    .flagEndDurationMs       = 10,
    .transmitCount           = 2,
    .delayMsBetweenTransmits = 3000,
};

static AppHABTracker1 app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


