#include "AppRgbLedSoundControl.h"


static AppRgbLedSoundControlConfig cfg = {
    .pinMicAnalogInput = 23
};

static AppRgbLedSoundControl app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


