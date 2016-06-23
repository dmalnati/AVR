#define IVM_DISABLE

#include "AppHABTracker1.h"


// Dummy values just to get it to compile
static AppHABTracker1Config cfg = {
    // SD Logger
    .pinSerialTxSdLogger = 4,

    // GPS
    .pinSerialRxGPS = 5,
    .pinSerialTxGPS = 6
};

static AppHABTracker1 app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


