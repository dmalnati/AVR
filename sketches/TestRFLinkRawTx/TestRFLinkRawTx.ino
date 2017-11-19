#include "PAL.h"
#include "RFLink.h"


static const uint8_t PIN_TX = 14;
RFLink_Raw rfr_;


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    uint8_t buf[20] = { '\0' };
    
    uint8_t seqNo = 0;

    rfr_.Init(-1, PIN_TX);
    
    while (1)
    {
        ++seqNo;
        
        Serial.print("Sending: ");
        Serial.println(seqNo);
        
        ltoa(seqNo, (char *)buf, 10);
        
        uint8_t bufLen = strlen((char *)buf);
        
        rfr_.Send((uint8_t *)buf, bufLen);

        PAL.Delay(1000);
    }
}

void loop() {}



