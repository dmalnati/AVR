#ifndef __APP_PICO_TRACKER_1_H__
#define __APP_PICO_TRACKER_1_H__


#include "Evm.h"
#include "AppPicoTracker1UserConfigManager.h"



struct AppPicoTracker1Config
{
};


class AppPicoTracker1
{
private:
    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;
    
public:

    
public:
    AppPicoTracker1(AppPicoTracker1Config &cfg)
    : cfg_(cfg)
    {
        // Nothing to do
    }
    
    void Run()
    {
        Serial.begin(9600);
        Serial.println(F("Starting"));
        
        if (AppPicoTracker1UserConfigManager::GetUserConfig(userConfig_))
        {
            Serial.println();
            Serial.println(F("Proceeding with:"));
            userConfig_.Print();
            Serial.println();
            
            
            
            
            
            Serial.println(F("Running."));
            Serial.println();
            PAL.Delay(1000);
            
            evm_.MainLoopLowPower();
        }
        else
        {
            Serial.println();
            Serial.println(F("ERR: Invalid configuration, please restart and try again."));
            Serial.println();
            
            // blink error -- cannot continue without config
        }
    }

    
    
    void ToDo()
    {
        
        
        
        // wtf were the bugs I had before?
            // Something about the deg min sec not accounting for 00 and
            // turning into just spaces right?
            
        // GPS geofencing to change frequency
    }
    
    
    
private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppPicoTracker1Config &cfg_;
    
    AppPicoTracker1UserConfigManager::UserConfig userConfig_;
};


#endif  // __APP_PICO_TRACKER_1_H__




























