#include "AppRgbLedAnalogControl.h"

static AppRgbLedAnalogControlConfig cfg = {
    // Red LED driven by OC1A - Pin 15
    .pinPotRed   = 23,

    // Green LED driven by OC1B - Pin 16
    .pinPotGreen = 24,

    // Blue LED driven by OC2A - Pin 17
    .pinPotBlue  = 25,
};

static AppRgbLedAnalogControl app(cfg);


void setup()
{
    app.Run();
}

void loop() {}




