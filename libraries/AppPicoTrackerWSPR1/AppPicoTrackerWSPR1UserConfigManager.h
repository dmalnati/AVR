#ifndef __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__
#define __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__


#include "SerialInput.h"
#include "WSPRMessageTransmitter.h"
#include "StrFormat.h"


struct AppPicoTrackerWSPR1UserConfig
{
    static const uint8_t ID_LEN          = 4;
    static const uint8_t CALLSIGN_ID_LEN = 2;
    
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
        //char callsign[CALLSIGN_ID_LEN + 1] = { 0 };
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
    } radio = {
        .mtCalibration = (WSPRMessageTransmitter::Calibration){
            -4500,
            7,
        }
    };
};


class AppPicoTrackerWSPR1UserConfigManager
: public PersistantConfigManager<AppPicoTrackerWSPR1UserConfig, 13, 0, 1>
{
public:

    AppPicoTrackerWSPR1UserConfigManager(uint8_t pinConfigure, AppPicoTrackerWSPR1UserConfig &config)
    : PersistantConfigManager(pinConfigure, config)
    {
        // Register formatter for uint32_t types which represent durations in ms
        idxFormatter_ = Menu().RegisterFormatter([](MenuType::Param &param){
            const char BUF_SIZE = 14;
            char buf[BUF_SIZE] = { 0 };
            
            DurationMsToHHMMSSmmm(*(uint32_t *)param.paramPtr, buf);
            
            LogNNL(buf);
        });
    }
    
private:

    void SetupMenu()
    {
        Menu().RegisterParamSTR(P("id"),                           Config().device.id, AppPicoTrackerWSPR1UserConfig::ID_LEN);
        
        Menu().RegisterParamSTR(P("callsignId"),                   Config().wspr.callsignId, AppPicoTrackerWSPR1UserConfig::CALLSIGN_ID_LEN);
        
        Menu().RegisterParamU8(P("solarMode"),                    &Config().power.solarMode);
        Menu().RegisterParamU16(P("minMilliVoltGpsLocationLock"), &Config().power.minMilliVoltGpsLocationLock);
        Menu().RegisterParamU16(P("minMilliVoltGpsTimeLock"),     &Config().power.minMilliVoltGpsTimeLock);
        Menu().RegisterParamU16(P("minMilliVoltTransmit"),        &Config().power.minMilliVoltTransmit);
        
        Menu().RegisterParamU32(P("gpsLockTimeoutMs"),            &Config().gps.gpsLockTimeoutMs, idxFormatter_);
        
        Menu().RegisterParamU32(P("lhAltFtThreshold"),            &Config().geo.lowHighAltitudeFtThreshold);
        Menu().RegisterParamU32(P("hAlt.wakeAndEvaluateMs"),      &Config().geo.highAltitude.wakeAndEvaluateMs, idxFormatter_);
        Menu().RegisterParamU32(P("lAlt.wakeAndEvaluateMs"),      &Config().geo.lowAltitude.wakeAndEvaluateMs, idxFormatter_);
        Menu().RegisterParamU32(P("lAlt.stickyMs"),               &Config().geo.lowAltitude.stickyMs, idxFormatter_);
        
        Menu().RegisterParamI32(P("crystalCorrectionFactor"),     &Config().radio.mtCalibration.crystalCorrectionFactor);
        Menu().RegisterParamI32(P("systemClockOffsetMs"),         &Config().radio.mtCalibration.systemClockOffsetMs);
    }
    
    int8_t idxFormatter_ = -1;
};


#endif  // __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__





















