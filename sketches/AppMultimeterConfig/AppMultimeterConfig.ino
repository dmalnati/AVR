#include "AppMultimeter.h"


static AppMultimeterConfig cfg =
{
    // Pins used for multiplexer to indicate channel
    .pinMuxBit0 = 11,
    .pinMuxBit1 = 12,
    .pinMuxBit2 = 13,
    .pinMuxBit3 = 14,

    // Pin doing actual ADC
    .pinAdc = 25,
};

static AppMultimeter app(cfg);


void setup()
{
    app.Run();
}

void loop() {}
