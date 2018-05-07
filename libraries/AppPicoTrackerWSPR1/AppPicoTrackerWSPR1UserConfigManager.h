#ifndef __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__
#define __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__


#include "WSPRMessageTransmitter.h"


class AppPicoTrackerWSPR1UserConfigManager
{
public:

    struct UserConfig
    {
        static const uint8_t CALLSIGN_LEN = 6;
        
        char *callsign[CALLSIGN_LEN + 1] = { 0 };
        
        WSPRMessageTransmitter::Calibration mtCalibration;
    };
    
    static uint8_t GetUserConfig(UserConfig &/*userConfig*/)
    {
        uint8_t retVal = 1;
        
        return retVal;
    }

};


#endif  // __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__









