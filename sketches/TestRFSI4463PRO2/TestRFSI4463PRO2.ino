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

void TestSetGetProperty()
{
    Serial.println("RequestDeviceState");



    uint16_t propertyList[] = { 0x0100, 0x0103, 0x0200, 0x0202 };

    for (auto property : propertyList)
    {
        uint8_t value;
    
        Serial.print("Property 0x");
        Serial.print(property, HEX);
        Serial.print(" = ");
        if (rf.GetProperty(property, value))
        {
            Serial.print("0b");
            Serial.println(value, BIN);
        }
        else
        {
            Serial.println("ERR");
        }
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
        //GetFifoInfo();
        RequestDeviceState();
        //TestSetGetProperty();

        PAL.Delay(2000);
    }
}


void loop() {}




