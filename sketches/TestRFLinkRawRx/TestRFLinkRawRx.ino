#include "PAL.h"
#include "UtlStreamBlob.h"
#include "Evm.h"
#include "RFLink.h"


static Evm::Instance<10,10,10> evm;

static const uint8_t PIN_RX = 15;
RFLink_Raw rfr_;


void OnMsgRcv(uint8_t *buf, uint8_t bufLen)
{
    Serial.print("Recvd ");
    Serial.print(bufLen);
    Serial.println();

    StreamBlob(Serial, buf, bufLen, 1, 1);
    Serial.println();
    Serial.println();
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    rfr_.SetOnMessageReceivedCallback([](uint8_t *buf, uint8_t bufLen){ OnMsgRcv(buf, bufLen); });
    rfr_.Init(PIN_RX, -1);
    
    
    evm.MainLoop();
}

void loop() {}



