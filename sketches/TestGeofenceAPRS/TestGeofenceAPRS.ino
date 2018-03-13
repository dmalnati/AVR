#include "PAL.h"
#include "Evm.h"
#include "UtlSerial.h"
#include "GeofenceAPRS.h"


static Evm::Instance<10,10,10>         evm;
static SerialAsyncConsoleEnhanced<10>  console;
static GeofenceAPRS                    geofence;


void Test(int32_t latitude, int32_t longitude)
{
    GeofenceAPRS::LocationDetails ld = geofence.GetLocationDetails(latitude, longitude);

    Serial.print("freq: ");
    Serial.print(ld.freqAprs);
    Serial.print(", deadZone: ");
    Serial.println(ld.deadZone);
}


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    console.RegisterCommand("test", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 3)
        {
            int32_t latitude  = atof(str.TokenAtIdx(1, ' ')) * 1000000;
            int32_t longitude = atof(str.TokenAtIdx(2, ' ')) * 1000000;

            Serial.print(F("Testing: "));
            Serial.print(latitude);
            Serial.print(" ");
            Serial.print(longitude);
            Serial.println();

            Test(latitude, longitude);
        }
    });

    console.Start();

    evm.MainLoop();
}

void loop() {}



