#include "AppHABTracker1Secondary.h"


static AppHABTracker1SecondaryConfig cfg =
{
    .pinRfTx = 14,
    
    .reportIntervalMs = 5000,
};

static AppHABTracker1Secondary app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


