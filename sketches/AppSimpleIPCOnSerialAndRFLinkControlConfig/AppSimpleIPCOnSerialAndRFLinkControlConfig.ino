#include "AppSimpleIPCOnSerialAndRFLinkControl.h"


static AppSimpleIPCOnSerialAndRFLinkControlConfig cfg = {
    .realm   = 1,
    .srcAddr = 9,
    .dstAddr = 0,

    .pinRFRX = 14,
    .pinRFTX = 15
};


static AppSimpleIPCOnSerialAndRFLinkControl app(cfg);


void setup() 
{
    app.Run();
}

void loop() { }


