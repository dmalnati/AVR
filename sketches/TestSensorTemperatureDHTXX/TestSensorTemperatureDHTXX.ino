#include "SensorTemperatureDHTXX.h"


static const uint8_t PIN_SENSOR = 11;


void setup()
{
    SensorTemperatureDHTXX s;
    s.Init(PIN_SENSOR);

    while (1)
    {
        SensorTemperatureDHTXX::Measurement m;

        m = s.GetMeasurement();
        
        PAL.Delay(2000);
    }
}

void loop() {}


