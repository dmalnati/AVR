#include "AppBikeLightComputer.h"

static AppBikeLightComputerConfig cfg =
{
    // interfacing config
    .pinConfigure = 6,

    // radio config
    .pinIrq = 12,
    .pinSdn = 13,
    .pinSel = 14,

    // initial settings
    .periodRedMs   = 10000,
    .periodGreenMs =  5000,
    .periodBlueMs  =  8000,
};

static AppBikeLightComputer app(cfg);


void setup()
{
    app.Run();
}

void loop() {}
