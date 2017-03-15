#include "SensorTemperatureDS18B20.h"

static const uint8_t PIN = 15;

static SensorTemperatureDS18B20 t(PIN);

void setup()
{
    Serial.begin(9600);

    t.Init();
    
    while (1)
    {
        SensorTemperatureDS18B20::Measurement m;

        uint8_t retVal = t.GetMeasurement(&m);

        if (retVal)
        {
            Serial.println("Success to read temp");
        }
        else
        {
            Serial.println("Failed to read temp");
        }
        
        
        PAL.Delay(500);
    }
}

void loop() {}




