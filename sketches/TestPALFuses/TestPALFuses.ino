#include "PAL.h"
#include "Log.h"




void PrintFuseInfo()
{
    Log("Fuse High: ", LogBIN(PAL.GetFuseHigh()));
    Log("Fuse Low : ", LogBIN(PAL.GetFuseLow()));
    Log("Fuse Ext : ", LogBIN(PAL.GetFuseExtended()));
    Log("Fuse Lock: ", LogBIN(PAL.GetFuseLock()));
    Log("BOD mV: ", PAL.GetFuseBODLimMilliVolts());

    LogNL();
}

void setup()
{
    LogStart(9600);
    Log("Starting");

    while (1)
    {
        PrintFuseInfo();
        PAL.Delay(1000);
    }
}

void loop() {}




