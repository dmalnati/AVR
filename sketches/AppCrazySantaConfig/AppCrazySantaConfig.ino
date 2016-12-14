#include "AppCrazySanta.h"


static AppCrazySantaConfig cfg = {
    .pinServoLeftArm  = 0,
    .pinServoRightArm = 0,
    .pinServoHead     = 0
};

static AppCrazySanta app(cfg);


void setup() 
{
    app.Run();
}

void loop() { }



