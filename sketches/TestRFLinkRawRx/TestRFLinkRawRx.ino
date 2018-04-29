#include "PAL.h"
#include "UtlStreamBlob.h"
#include "Evm.h"
#include "RFLink.h"


static Evm::Instance<10,10,10> evm;

static const uint8_t PIN_RX = 15;
RFLink_Raw rfr_;


void OnMsgRcv(uint8_t *buf, uint8_t bufLen)
{
    @fix@Serial.print("Recvd ");
    @fix@Serial.print(bufLen);
    @fix@Serial.println();

    StreamBlob(Serial, buf, bufLen, 1, 1);
    @fix@Serial.println();
    @fix@Serial.println();
}

void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    rfr_.SetOnMessageReceivedCallback([](uint8_t *buf, uint8_t bufLen){ OnMsgRcv(buf, bufLen); });
    rfr_.Init(PIN_RX, -1);
    
    
    evm.MainLoop();
}

void loop() {}



