#include "AppBikeLightComputer.h"

static AppBikeLightComputerRemoteConfig cfg =
{
    .cfg = {
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
    },
};

#if 0
static AppBikeLightComputer app(cfg.cfg);
#else
static AppBikeLightComputerRemote app(cfg);
#endif

void setup()
{
    app.Run();
}

void loop() {}
