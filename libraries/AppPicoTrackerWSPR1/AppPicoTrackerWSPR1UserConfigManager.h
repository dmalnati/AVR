#ifndef __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__
#define __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__


#include "SerialInput.h"
#include "WSPRMessageTransmitter.h"


struct AppPicoTrackerWSPR1UserConfig
{
    static const uint8_t ID_LEN       = 4;
    static const uint8_t CALLSIGN_LEN = 6;
    
    struct
    {
        //char id[ID_LEN + 1] = { 0 };
        char id[ID_LEN + 1] = "DM01";
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
        //char callsign[CALLSIGN_LEN + 1] = { 0 };
        char callsign[CALLSIGN_LEN + 1] = "KD2KDD";
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
        uint8_t tcxoMode = 0;
    } clock;
    
    struct
    {
        WSPRMessageTransmitter::Calibration mtCalibration;
    } radio = {
        .mtCalibration = (WSPRMessageTransmitter::Calibration){
            -3652,
            3,
        }
    };
};


class AppPicoTrackerWSPR1UserConfigManager
: public PersistantConfigManager<AppPicoTrackerWSPR1UserConfig, 14>
{
public:

    AppPicoTrackerWSPR1UserConfigManager(uint8_t pinConfigure, AppPicoTrackerWSPR1UserConfig &config)
    : PersistantConfigManager(pinConfigure, config)
    {
        // Nothing to do
    }
    
private:

    void SetupMenu()
    {
        Menu().RegisterParamSTR(P("id"),                           Config().device.id, AppPicoTrackerWSPR1UserConfig::ID_LEN);
        
        Menu().RegisterParamSTR(P("callsign"),                     Config().wspr.callsign, AppPicoTrackerWSPR1UserConfig::CALLSIGN_LEN);
        
        Menu().RegisterParamU8(P("solarMode"),                    &Config().power.solarMode);
        Menu().RegisterParamU16(P("minMilliVoltGpsLocationLock"), &Config().power.minMilliVoltGpsLocationLock);
        Menu().RegisterParamU16(P("minMilliVoltGpsTimeLock"),     &Config().power.minMilliVoltGpsTimeLock);
        Menu().RegisterParamU16(P("minMilliVoltTransmit"),        &Config().power.minMilliVoltTransmit);
        
        Menu().RegisterParamU32(P("gpsLockTimeoutMs"),            &Config().gps.gpsLockTimeoutMs);
        
        Menu().RegisterParamU32(P("lhAltFtThreshold"),            &Config().geo.lowHighAltitudeFtThreshold);
        Menu().RegisterParamU32(P("hAlt.wakeAndEvaluateMs"),      &Config().geo.highAltitude.wakeAndEvaluateMs);
        Menu().RegisterParamU32(P("lAlt.wakeAndEvaluateMs"),      &Config().geo.lowAltitude.wakeAndEvaluateMs);
        Menu().RegisterParamU32(P("lAlt.stickyMs"),               &Config().geo.lowAltitude.stickyMs);
        
        Menu().RegisterParamU8(P("tcxoMode"),                     &Config().clock.tcxoMode);
        Menu().RegisterParamI32(P("systemClockOffsetMs"),         &Config().radio.mtCalibration.systemClockOffsetMs);
        
        Menu().RegisterParamI32(P("crystalCorrectionFactor"),     &Config().radio.mtCalibration.crystalCorrectionFactor);
    }
};


#endif  // __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__





















