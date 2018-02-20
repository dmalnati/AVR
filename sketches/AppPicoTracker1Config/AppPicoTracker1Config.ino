#include "AppPicoTracker1.h"


static AppPicoTracker1Config cfg =
{
    .pinGpsEnable = 26,
    
    .pinLedRunning      = 13,
    .pinLedGpsLocked    = 12,
    .pinLedTransmitting = 11,
};

static AppPicoTracker1 app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


