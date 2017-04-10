#include "PAL.h"
#include "SensorSoilMoisture.h"


static const uint8_t PIN_POWER       = 16;
static const uint8_t PIN_MEASUREMENT = 25;

//static SensorSoilMoisture s(PIN_POWER, PIN_MEASUREMENT);


void setup()
{
    Serial.begin(9600);

    SensorSoilMoisture s(PIN_POWER, PIN_MEASUREMENT);
    SensorSoilMoisture::Measurement m;

    while (1)
    {
        if (s.GetMeasurement(&m))
        {
            Serial.println("Measurement successful");
            
            Serial.print("pctConductive: ");
            Serial.println(m.pctConductive);
        }
        else
        {
            Serial.println("Measurement failure");
        }
    
        Serial.println();
    
        PAL.Delay(1000);

    }
}

void loop() {}




