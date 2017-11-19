#include "AppMMToGpsLCD.h"


static AppMMToGpsLCDConfig cfg =
{
    // APRS Modem configuration
    .baud       = 85000,
    .callsign = "KD2KDD",

    // 433MHz configuration
    .pinRfRx = 14,

    // Circuit configuration
    .i2cAddrLcd = 0x27,
};

static AppMMToGpsLCD app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


