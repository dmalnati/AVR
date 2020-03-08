#ifndef __APP_PICO_TRACKER_WSPR_2_USER_CONFIG_H__
#define __APP_PICO_TRACKER_WSPR_2_USER_CONFIG_H__


#include "WSPRMessageTransmitter.h"


struct AppPicoTrackerWSPR2UserConfig
{
    static const uint8_t ID_LEN          = 4;
    static const uint8_t CALLSIGN_LEN    = 6;
    static const uint8_t CALLSIGN_ID_LEN = 2;
    
    struct
    {
        char id[ID_LEN + 1] = { 0 };
    } device;
    
    // WSPR Stuff
    struct
    {
        char callsign[CALLSIGN_LEN + 1]      = "      ";
        char callsignId[CALLSIGN_ID_LEN + 1] = "00";
    } wspr;
    
    // Tracker stuff
    struct
    {
        uint32_t lowHighAltitudeFtThreshold = 25000;
        
        struct
        {
            uint32_t wakeAndEvaluateMs = 600000UL;    // 10 min
        } highAltitude;
        
        struct
        {
            uint32_t wakeAndEvaluateMs =  60000UL;   // 60 sec, 1 min
        } lowAltitude;
    } geo;
    
    struct
    {
        WSPRMessageTransmitter::Calibration mtCalibration;
    } radio;
};


#endif  // __APP_PICO_TRACKER_WSPR_2_USER_CONFIG_H__





































