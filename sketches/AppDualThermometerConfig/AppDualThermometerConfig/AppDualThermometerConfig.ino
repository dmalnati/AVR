#include "AppDualThermometer.h"


static AppDualThermometerConfig cfg = {
    .pinThermometer1 = 15,
    .pinServo1       = 16,
    
    .pinThermometer2 = 14,
    .pinServo2       = 13
};

static AppDualThermometer app(cfg);


void setup()
{
    app.Run();
}

void loop() { }



