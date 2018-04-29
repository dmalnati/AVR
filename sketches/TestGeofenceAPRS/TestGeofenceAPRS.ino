#include "PAL.h"
#include "Evm.h"
#include "Utl@fix@Serial.h"
#include "GeofenceAPRS.h"


static Evm::Instance<10,10,10>         evm;
static SerialAsyncConsoleEnhanced<10>  console;
static GeofenceAPRS                    geofence;


void Test(int32_t latitude, int32_t longitude)
{
    GeofenceAPRS::LocationDetails ld = geofence.GetLocationDetails(latitude, longitude);

    @fix@Serial.print("freq: ");
    @fix@Serial.print(ld.freqAprs);
    @fix@Serial.print(", deadZone: ");
    @fix@Serial.println(ld.deadZone);
}


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    console.RegisterCommand("test", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 3)
        {
            int32_t latitude  = atof(str.TokenAtIdx(1, ' ')) * 1000000;
            int32_t longitude = atof(str.TokenAtIdx(2, ' ')) * 1000000;

            @fix@Serial.print(F("Testing: "));
            @fix@Serial.print(latitude);
            @fix@Serial.print(" ");
            @fix@Serial.print(longitude);
            @fix@Serial.println();

            Test(latitude, longitude);
        }
    });

    console.Start();

    evm.MainLoop();
}

void loop() {}



