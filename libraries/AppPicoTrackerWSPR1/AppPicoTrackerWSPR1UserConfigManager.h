#ifndef __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__
#define __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__


#include "SerialInput.h"
#include "WSPRMessageTransmitter.h"
#include "StrFormat.h"
#include "AppPicoTrackerWSPR1UserConfig.h"


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





















