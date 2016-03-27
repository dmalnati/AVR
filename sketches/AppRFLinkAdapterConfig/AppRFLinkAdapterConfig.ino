#include <AppRFLinkAdapter.h>


static AppRFLinkAdapterConfig cfg = {
    .pinSerialRX = 2,
    .pinSerialTX = 3,

    .pinRFRX = 14,
    .pinRFTX = 15,

    .pinLEDRX = 13,
    .pinLEDTX = 16
};

static AppRFLinkAdapter app(cfg);


void setup()
{
    app.Run();
}

void loop() {}



