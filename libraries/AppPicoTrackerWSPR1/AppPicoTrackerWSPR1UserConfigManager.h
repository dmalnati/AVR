#ifndef __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__
#define __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__


#include "WSPRMessageTransmitter.h"


class AppPicoTrackerWSPR1UserConfigManager
{
public:

    struct UserConfig
    {
        // Solar vs battery related
        uint8_t solarMode = 0;
        
        uint16_t minMilliVoltGpsLocationLock = 0;
        uint16_t minMilliVoltGpsTimeLock     = 0;
        uint16_t minMilliVoltTransmit        = 0;
        
        // GPS stuff
        uint32_t gpsLockTimeoutMs = 70000;
        
        // WSPR Stuff
        static const uint8_t CALLSIGN_LEN = 6;
        
        char *callsign[CALLSIGN_LEN + 1] = { 0 };
        
        WSPRMessageTransmitter::Calibration mtCalibration;
    };
    
    static uint8_t GetUserConfig(uint8_t pinConfigure, UserConfig &/*userConfig*/)
    {
        uint8_t retVal = 1;
        
        // Check the pinConfigure pin for a signal that a human is wanting to
        // do configuration.
        // Set pin as a pullup, and sense if pin is low as that signal.
        PAL.PinMode(pinConfigure, INPUT_PULLUP);
        
        return retVal;
    }

};


#endif  // __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__









