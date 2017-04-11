#include "SignalProcessingFHT.h"


static SignalProcessingFHT fht;


void setup()
{
    Serial.begin(9600);

    Pin pinAnalog(23);

    while (1)
    {
        SignalProcessingFHT::Measurement m;

        if (fht.GetMeasurement(pinAnalog, &m))
        {
            // Send data
            Serial.print("valListLen: "); Serial.println(m.valListLen);
    
            for (uint16_t i = 0; i < m.valListLen/2; ++i)
            {
                Serial.print(i);
                Serial.print(": ");
                Serial.println(m.valList[i]);
            }
            Serial.println();
        }
    }
}

void loop() {}




