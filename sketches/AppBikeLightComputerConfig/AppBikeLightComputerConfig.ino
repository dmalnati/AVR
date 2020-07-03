#include "AppBikeLightComputer.h"

static AppBikeLightComputerConfig cfg =
{
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
