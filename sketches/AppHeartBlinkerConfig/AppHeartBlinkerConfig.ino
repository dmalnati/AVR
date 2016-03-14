#include "AppHeartBlinker.h"


static AppHeartBlinkerConfig cfg = {
    .pinLED1  =  5,
    .pinLED2  =  4,
    .pinLED3  =  3,
    .pinLED4  =  6,
    .pinLED5  = 13,
    .pinLED6  = 14,
    .pinLED7  = 15,
    .pinLED8  =  2,
    .pinLED9  = 28,
    .pinLED10 = 16,
    .pinLED11 = 17,
    .pinLED12 = 27,
    .pinLED13 = 26,
    .pinLED14 = 23,
    .pinLED15 = 18,
    .pinLED16 = 19
};

static AppHeartBlinker app(cfg);



void setup()
{
    app.Run();
}

void loop() {}
