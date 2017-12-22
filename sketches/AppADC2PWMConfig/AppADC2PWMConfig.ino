#include "AppADC2PWM.h"


static AppADC2PWMConfig cfg =
{
    .pinAdc = 23,
};

static AppADC2PWM app(cfg);


void setup()
{
    app.Run();
}


void loop() {}




