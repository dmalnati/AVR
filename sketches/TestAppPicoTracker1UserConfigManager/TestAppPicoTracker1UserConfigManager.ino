#include "PAL.h"
#include "AppPicoTracker1UserConfigManager.h"


static AppPicoTracker1UserConfigManager::UserConfig userConfig;


void Test()
{
    uint8_t retVal = AppPicoTracker1UserConfigManager::GetUserConfig(userConfig);

    Serial.print("retVal: "); Serial.println(retVal);
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");
    
    while (1)
    {
        Test();

        Serial.println();
        Serial.println();

        PAL.Delay(1000);
    }
}

void loop(){}


