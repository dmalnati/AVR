#include "AppCrazySanta.h"


static AppCrazySantaConfig cfg = {
    .pinServoLeftArm  = 4,
    .pinServoRightArm = 5,
    .pinServoHead     = 6,

    .pinLedLeftEye  = 10,
    .pinLedRightEye = 11
};

static AppCrazySanta app(cfg);


void setup() 
{
    app.Run();
}

void loop() { }



