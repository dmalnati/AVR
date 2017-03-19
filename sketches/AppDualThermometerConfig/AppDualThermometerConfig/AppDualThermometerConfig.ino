#include "AppDualThermometer.h"


static AppDualThermometerConfig cfg = {
    .pinThermometer1                 = 14,
    .pinServo1                       = 13,
    .servo1PhysicalCalibrationOffset = -8,
    
    .pinThermometer2                 = 15,
    .pinServo2                       = 16,
    .servo2PhysicalCalibrationOffset = -8
};

static AppDualThermometer app(cfg);


void setup()
{
    app.Run();
}

void loop() { }



