#include "PAL.h"
#include "RFLink.h"


static const uint8_t RF_REALM    = 0;
static const uint8_t RF_SRC_ADDR = 1;
static const uint8_t RF_DST_ADDR = 2;
static const uint8_t PROTOCOL_ID = 0;

static const uint8_t PIN_TX = 14;
RFLink rf_;


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    uint8_t buf[20] = { '\0' };
    
    uint8_t seqNo = 0;

    rf_.SetRealm(RF_REALM);
    rf_.SetSrcAddr(RF_SRC_ADDR);
    rf_.SetDstAddr(RF_DST_ADDR);
    rf_.Init(-1, PIN_TX);
    
    while (1)
    {
        ++seqNo;
        
        Serial.print("Sending: ");
        Serial.println(seqNo);
        
        ltoa(seqNo, (char *)buf, 10);
        
        uint8_t bufLen = strlen((char *)buf);
        
        rf_.Send(PROTOCOL_ID, (uint8_t *)buf, bufLen);

        PAL.Delay(1000);
    }
}

void loop() {}



