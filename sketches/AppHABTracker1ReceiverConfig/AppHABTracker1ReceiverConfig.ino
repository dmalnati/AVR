#include "AppHABTracker1Receiver.h"


static AppHABTracker1ReceiverConfig cfg =
{
    // APRS Modem configuration
    .baud       = 85000,
    .callsign = "KD2KDD",

    // 433MHz configuration
    .pinRfRx = 14,

    // Circuit configuration
    .i2cAddrLcd = 0x27,
};

static AppHABTracker1Receiver app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


