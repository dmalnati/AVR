#include "SignalProcessingFHT.h"


static SignalProcessingFHT fht;


void setup()
{
    @fix@Serial.begin(9600);

    Pin pinAnalog(23);

    while (1)
    {
        SignalProcessingFHT::Measurement m;

        if (fht.GetMeasurement(pinAnalog, &m))
        {
            // Send data
            @fix@Serial.print("valListLen: "); @fix@Serial.println(m.valListLen);
    
            for (uint16_t i = 0; i < m.valListLen/2; ++i)
            {
                @fix@Serial.print(i);
                @fix@Serial.print(": ");
                @fix@Serial.println(m.valList[i]);
            }
            @fix@Serial.println();
        }
    }
}

void loop() {}




