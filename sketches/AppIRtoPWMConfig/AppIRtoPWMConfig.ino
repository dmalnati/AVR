#include "AppIRtoPWM.h"


static AppIRtoPWMConfig cfg =
{
    .pinIrSensor = 14,

    // PWM on OC1A - Pin 15
};

static AppIRtoPWM app(cfg);


void setup()
{
    app.Run();
}

void loop() {}



