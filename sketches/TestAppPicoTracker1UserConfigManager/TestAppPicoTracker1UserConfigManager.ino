#include "PAL.h"
#include "AppPicoTracker1UserConfigManager.h"


static AppPicoTracker1UserConfigManager::UserConfig userConfig;


void Test()
{
    uint8_t retVal = AppPicoTracker1UserConfigManager::GetUserConfig(userConfig);

    @fix@Serial.print("retVal: "); @fix@Serial.println(retVal);
}

void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");
    
    while (1)
    {
        Test();

        @fix@Serial.println();
        @fix@Serial.println();

        PAL.Delay(1000);
    }
}

void loop(){}


