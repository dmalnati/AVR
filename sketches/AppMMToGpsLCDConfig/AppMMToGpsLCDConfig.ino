#include "AppMMToGpsLCD.h"


static AppMMToGpsLCDConfig cfg =
{
    .baud       = 85000,
    .i2cAddrLcd = 0x27,
};

static AppMMToGpsLCD app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


