#include "AppRgbLedRemoteControl.h"


static Config cfg =
{
};


#if 1
    static AppRgbLedRemoteControlMaster app(cfg);
#else
    static AppRgbLedRemoteControlSlave app(cfg);
#endif


void setup()
{
    app.Run();
}

void loop() {}
