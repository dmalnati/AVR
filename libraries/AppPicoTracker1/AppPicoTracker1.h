#ifndef __APP_PICO_TRACKER_1_H__
#define __APP_PICO_TRACKER_1_H__


#include "Evm.h"



struct AppPicoTracker1Config
{
};

struct UserConfig
{
public:
    UserConfig()
    {
        memset((void *)callsign, '\0', 7);
        memset((void *)notesMessage, '\0', 25);
    }
    
    void SetCallsign(char *str)
    {
        strncpy(callsign, str, 7);
        callsign[6] = '\0';
    }
    
    char    callsign[7];
    uint8_t ssid = 9;
    
    uint32_t reportIntervalSecs = 60;
    
    uint8_t  transmitCount             = 2;
    uint32_t delaySecsBetweenTransmits = 3;
    
    char notesMessage[25];
};

#include "AppPicoTracker1UserConfigSerialInterface.h"

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
        Serial.println("Starting");
        
        PAL.Delay(20);
        
        if (PAL.DigitalRead(2))
        {
            Serial.println("Config mode enabled");
            
            AppPicoTracker1UserConfigSerialInterface menu;
            
            menu.GetUserConfig();
        }
        else
        {
            Serial.println("Config mode not in use");
        }
        
        while (1) {}
        
        // blink error if no valid configuration found
        
        
        
        // wtf were the bugs I had before?
            // Something about the deg min sec not accounting for 00 and
            // turning into just spaces right?
    }

private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppPicoTracker1Config &cfg_;
};


#endif  // __APP_PICO_TRACKER_1_H__