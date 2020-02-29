#ifndef __APP_PICO_TRACKER_WSPR_2_USER_CONFIG_H__
#define __APP_PICO_TRACKER_WSPR_2_USER_CONFIG_H__


#include "WSPRMessageTransmitter.h"


struct AppPicoTrackerWSPR2UserConfig
{
    static const uint8_t ID_LEN          = 4;
    static const uint8_t CALLSIGN_ID_LEN = 2;
    
    struct
    {
        char id[ID_LEN + 1] = { 0 };
    } device;
    
    // Solar vs battery related
    struct
    {
        uint8_t solarMode = 0;
        
        uint16_t minMilliVoltGpsLocationLock = 0;
        uint16_t minMilliVoltGpsTimeLock     = 0;
        uint16_t minMilliVoltTransmit        = 0;
    } power;
    
    // GPS stuff
    struct
    {
        uint32_t gpsLockTimeoutMs = 150000UL; // 2m30 - 85th percentile cold start success
    } gps;
    
    // WSPR Stuff
    struct
    {
        char callsignId[CALLSIGN_ID_LEN + 1] = "00";
    } wspr;
    
    // Tracker stuff
    struct
    {
        uint32_t lowHighAltitudeFtThreshold = 10000;
        
        struct
        {
            uint32_t wakeAndEvaluateMs = 30L * 60L * 1000L;    // 30 min
        } highAltitude;
        
        struct
        {
            uint32_t wakeAndEvaluateMs =        60L * 1000L;   // 60 sec, 1 min
            uint32_t stickyMs          = 180L * 60L * 1000L;   // 3 hours
        } lowAltitude;
    } geo;
    
    struct
    {
        WSPRMessageTransmitter::Calibration mtCalibration;
    } radio;
};


#endif  // __APP_PICO_TRACKER_WSPR_2_USER_CONFIG_H__





































