#include "AppAPRSISS.h"


static AppAPRSISSConfig cfg =
{
    // Human interfacing
    .pinConfigure = 15,
    .pinSend = 15,
    .pinFreqSelect = 16,
    
    // GPS
    .pinGpsSerialRx = 23,
    .pinGpsSerialTx = 24,

    // Status LEDs
    .pinLedRed   = 12,
    .pinLedGreen = 13,

    // Radio
    .pinRadioSlaveSelect = 25,
    .pinRadioShutdown    = 14,
};

static AppAPRSISS app(cfg);


void setup()
{
    app.Run();
}

void loop() {}
