#include "RFSI4463PRO.h"
#include "RFSI4463PRODebug.h"

static const uint8_t PIN_SS       = 15;
static const uint8_t PIN_SHUTDOWN = 14;

static RFSI4463PRO rf(PIN_SS, PIN_SHUTDOWN);
static RFSI4463PRODebug rfd;


void GetPartInfo()
{
    Serial.println("GetPartInfo");
    
    RFSI4463PRO::PartInfo partInfo;

    if (rf.GetPartInfo(partInfo))
    {
        Serial.print("chipRevision: "); Serial.println(partInfo.chipRevision);
        Serial.print("partNumber  : "); Serial.println(partInfo.partNumber);
        Serial.print("partBuild   : "); Serial.println(partInfo.partBuild);
        Serial.print("id          : "); Serial.println(partInfo.id);
        Serial.print("customerId  : "); Serial.println(partInfo.customerId);
        Serial.print("romId       : "); Serial.println(partInfo.romId);
    }
    else
    {
        Serial.println("ERR");
    }

    Serial.println();
}

void GetPartInfo2()
{
    Serial.println("GetPartInfo2");
    
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


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    rf.Init();

    while (1)
    {
        GetPartInfo();
        GetPartInfo2();
        GetChipStatus();


        PAL.Delay(5000);
    }
}


void loop() {}




