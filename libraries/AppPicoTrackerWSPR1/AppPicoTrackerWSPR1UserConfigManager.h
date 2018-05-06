#ifndef __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__
#define __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__


#include "WSPRMessageTransmitter.h"


class AppPicoTrackerWSPR1UserConfigManager
{
public:

    struct UserConfig
    {
        WSPRMessageTransmitter::Calibration mtCalibration;
        
        // help the tracker guess better about how early to wake up
        uint32_t expectedGpsLockDurationMs = 45000;
    };
    
    static uint8_t GetUserConfig(UserConfig &userConfig)
    {
        uint8_t retVal = 0;
        
        userConfig;

        return retVal;
    }

};


#endif  // __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__









