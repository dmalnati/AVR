#include "RFSI4463PRO.h"
#include "RFSI4463PRODebug.h"

static const uint8_t PIN_SS       = 15;
static const uint8_t PIN_SHUTDOWN = 14;

static RFSI4463PRO rf(PIN_SS, PIN_SHUTDOWN);
static RFSI4463PRODebug rfd;


void GetPartInfo()
{
    Serial.println("GetPartInfo");
    
    RFSI4463PRO::PART_INFO_REP retVal;

    if (rf.Command_PART_INFO(retVal))
    {
        rfd.Print(retVal);
    }
    else
    {
        Serial.println("ERR");
    }

    Serial.println();
}

void GetFuncInfo()
{
    Serial.println("GetFuncInfo");
    
    RFSI4463PRO::FUNC_INFO_REP retVal;

    if (rf.Command_FUNC_INFO(retVal))
    {
        rfd.Print(retVal);
    }
    else
    {
        Serial.println("ERR");
    }

    Serial.println();
}

void GetChipStatus()
{
    Serial.println("GetChipStatus");
    
    RFSI4463PRO::GET_CHIP_STATUS_REP retVal;

    if (rf.Command_GET_CHIP_STATUS(retVal))
    {
        rfd.Print(retVal);
    }
    else
    {
        Serial.println("ERR");
    }

    Serial.println();
}

void GetFifoInfo()
{
    Serial.println("GetFifoInfo");
    
    RFSI4463PRO::FIFO_INFO_REQ req;
    RFSI4463PRO::FIFO_INFO_REP rep;

    if (rf.Command_FIFO_INFO(req, rep))
    {
        rfd.Print(rep);
    }
    else
    {
        Serial.println("ERR");
    }

    Serial.println();
}

void RequestDeviceState()
{
    Serial.println("RequestDeviceState");
    
    RFSI4463PRO::REQUEST_DEVICE_STATE_REP rep;

    if (rf.Command_REQUEST_DEVICE_STATE(rep))
    {
        rfd.Print(rep);
    }
    else
    {
        Serial.println("ERR");
    }

    Serial.println();
}


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    rf.Init();

    while (1)
    {
        GetPartInfo();
        GetFuncInfo();
        GetChipStatus();
        GetFifoInfo();
        RequestDeviceState();

        PAL.Delay(5000);
    }
}


void loop() {}




