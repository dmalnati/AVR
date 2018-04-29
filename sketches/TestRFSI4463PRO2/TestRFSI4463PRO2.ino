#include "RFSI4463PRO.h"
#include "RFSI4463PRODebug.h"

static const uint8_t PIN_SS       = 25;
static const uint8_t PIN_SHUTDOWN = 14;

static RFSI4463PRO rf(PIN_SS, PIN_SHUTDOWN);
static RFSI4463PRODebug rfd;


void GetPartInfo()
{
    @fix@Serial.println("GetPartInfo");
    
    RFSI4463PRO::PART_INFO_REP retVal;

    if (rf.Command_PART_INFO(retVal))
    {
        rfd.Print(retVal);
    }
    else
    {
        @fix@Serial.println("ERR");
    }

    @fix@Serial.println();
}

void GetFuncInfo()
{
    @fix@Serial.println("GetFuncInfo");
    
    RFSI4463PRO::FUNC_INFO_REP retVal;

    if (rf.Command_FUNC_INFO(retVal))
    {
        rfd.Print(retVal);
    }
    else
    {
        @fix@Serial.println("ERR");
    }

    @fix@Serial.println();
}

void GetChipStatus()
{
    @fix@Serial.println("GetChipStatus");
    
    RFSI4463PRO::GET_CHIP_STATUS_REP retVal;

    if (rf.Command_GET_CHIP_STATUS(retVal))
    {
        rfd.Print(retVal);
    }
    else
    {
        @fix@Serial.println("ERR");
    }

    @fix@Serial.println();
}

void RequestDeviceState()
{
    @fix@Serial.println("RequestDeviceState");
    
    RFSI4463PRO::REQUEST_DEVICE_STATE_REP rep;

    if (rf.Command_REQUEST_DEVICE_STATE(rep))
    {
        rfd.Print(rep);
    }
    else
    {
        @fix@Serial.println("ERR");
    }

    @fix@Serial.println();
}


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    rf.Init();
    rf.Start();

    while (1)
    {
        GetPartInfo();
        GetFuncInfo();
        RequestDeviceState();
        
        GetChipStatus();
        
        
        @fix@Serial.println();
        PAL.Delay(2000);
    }
}


void loop() {}




