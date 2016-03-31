#include "SensorTemperatureDHTXX.h"
#include "PAL.h"


static const uint8_t PIN_SENSOR = 11;

static SensorTemperatureDHTXX              s(PIN_SENSOR);
static SensorTemperatureDHTXX::Measurement m;


void setup()
{
    s.Init();
    
    while (1)
    {
        // Sensor will try up to 3 times unless default overridden.
        // This is fine for testing purposes
        // (and probably most others).
        s.GetMeasurement(&m);

        // This is about as low as it went.
        // The only other lower number tried was 100ms.
        PAL.Delay(150);
    }
}

void loop() {}


