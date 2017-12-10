#include "RFSI4463PRO.h"

static const uint8_t PIN_SS       = 15;
static const uint8_t PIN_SHUTDOWN = 14;

static RFSI4463PRO rf(PIN_SS, PIN_SHUTDOWN);


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


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    rf.Init();

    while (1)
    {
        GetPartInfo();


        PAL.Delay(5000);
    }
}


void loop() {}




