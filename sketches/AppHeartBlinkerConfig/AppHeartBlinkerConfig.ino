#include "AppHeartBlinker.h"


static AppHeartBlinkerConfig cfg = {
    //
    // During Eagle placement, resistors and LEDs wound up out of sequence to original intent.
    //
    // The correct logical numbering scheme starts with LED1 being the top middle heart
    // LED, and going clockwise from there.
    //
    .pinLED1  =  6,
    .pinLED2  = 13,
    .pinLED3  = 14,
    .pinLED4  = 15,
    .pinLED5  = 16,
    .pinLED6  = 17,
    .pinLED7  = 18,
    .pinLED8  = 19,
    .pinLED9  = 23,
    .pinLED10 = 26,
    .pinLED11 = 27,
    .pinLED12 = 28,
    .pinLED13 =  2,
    .pinLED14 =  3,
    .pinLED15 =  4,
    .pinLED16 =  5
};

static AppHeartBlinker app(cfg);



void setup()
{
    app.Run();
}

void loop() {}
