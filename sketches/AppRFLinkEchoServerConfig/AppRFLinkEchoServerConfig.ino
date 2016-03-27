#include "AppRFLinkEchoServer.h"


static AppRFLinkEchoServerConfig cfg = {
    .pinRFRX = 14,
    .pinRFTX = 15,

    .pinLEDRX = 13,
    .pinLEDTX = 16,

    .realm   = 1,
    .srcAddr = 1
};

static AppRFLinkEchoServer app(cfg);


void setup()
{
    app.Run();
}

void loop() {}


