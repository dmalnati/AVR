#include "PAL.h"
#include "UtlStreamBlob.h"
#include "Evm.h"
#include "RFLink.h"


static Evm::Instance<10,10,10> evm;

static const uint8_t RF_REALM    = 0;
static const uint8_t RF_SRC_ADDR = 2;
static const uint8_t RF_DST_ADDR = 1;
static const uint8_t PROTOCOL_ID = 0;

static const uint8_t PIN_RX = 15;
RFLink rf_;


void OnMsgRcv(RFLinkHeader *hdr, uint8_t *buf, uint8_t bufLen)
{
    @fix@Serial.print("Recvd ");
    @fix@Serial.print(bufLen);
    @fix@Serial.println();
    
    @fix@Serial.print("Realm  : "); @fix@Serial.println(hdr->realm);
    @fix@Serial.print("Src    : "); @fix@Serial.println(hdr->srcAddr);
    @fix@Serial.print("Dst    : "); @fix@Serial.println(hdr->dstAddr);
    @fix@Serial.print("ProtoID: "); @fix@Serial.println(hdr->protocolId);

    StreamBlob(Serial, buf, bufLen, 1, 1);
    @fix@Serial.println();
    @fix@Serial.println();
}

void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    rf_.SetRealm(RF_REALM);
    rf_.SetSrcAddr(RF_SRC_ADDR);
    rf_.SetDstAddr(RF_DST_ADDR);
    rf_.SetOnMessageReceivedCallback([](RFLinkHeader *hdr, uint8_t *buf, uint8_t bufLen){ OnMsgRcv(hdr, buf, bufLen); });
    rf_.Init(PIN_RX, -1);
    
    
    evm.MainLoop();
}

void loop() {}



