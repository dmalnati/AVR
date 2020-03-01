#ifndef __APP_PICO_TRACKER_WSPR_2_USER_CONFIG_MANAGER_H__
#define __APP_PICO_TRACKER_WSPR_2_USER_CONFIG_MANAGER_H__


#include "SerialInput.h"
#include "WSPRMessageTransmitter.h"
#include "StrFormat.h"
#include "AppPicoTrackerWSPR2UserConfig.h"


class AppPicoTrackerWSPR2UserConfigManager
: public PersistantConfigManager<AppPicoTrackerWSPR2UserConfig, 8, 0, 1>
{
public:

    AppPicoTrackerWSPR2UserConfigManager(uint8_t pinConfigure, AppPicoTrackerWSPR2UserConfig &config)
    : PersistantConfigManager(pinConfigure, config)
    {
        // Register formatter for uint32_t types which represent durations in ms
        idxFormatter_ = Menu().RegisterFormatter([](MenuType::Param &param){
            char buf[StrFormat::HHMMSSMMM_BUF_SIZE_NEEDED] = { 0 };
            
            StrFormat::DurationMsToHHMMSSMMM(*(uint32_t *)param.paramPtr, buf);
            StrFormat::TrimMsFromHHMMSSMMM(buf);
            StrFormat::TrimLeadingPaddingFromHHMMSSMMM(buf);
            
            LogNNL(buf);
        });
    }
    
private:

    void SetupMenu()
    {
        Menu().RegisterParamSTR(P("trackerId"),                    Config().device.id, AppPicoTrackerWSPR2UserConfig::ID_LEN);
        
        Menu().RegisterParamSTR(P("wsprCallsign"),                 Config().wspr.callsign, AppPicoTrackerWSPR2UserConfig::CALLSIGN_LEN);
        Menu().RegisterParamSTR(P("wsprCallsignId"),               Config().wspr.callsignId, AppPicoTrackerWSPR2UserConfig::CALLSIGN_ID_LEN);
        
        Menu().RegisterParamU32(P("lhAltFtThreshold"),            &Config().geo.lowHighAltitudeFtThreshold);
        Menu().RegisterParamU32(P("hAlt.wakeAndEvaluateMs"),      &Config().geo.highAltitude.wakeAndEvaluateMs, idxFormatter_);
        Menu().RegisterParamU32(P("lAlt.wakeAndEvaluateMs"),      &Config().geo.lowAltitude.wakeAndEvaluateMs, idxFormatter_);
        
        Menu().RegisterParamI32(P("crystalCorrectionFactor"),     &Config().radio.mtCalibration.crystalCorrectionFactor);
        Menu().RegisterParamI32(P("systemClockOffsetMs"),         &Config().radio.mtCalibration.systemClockOffsetMs);
    }
    
    int8_t idxFormatter_ = -1;
};


#endif  // __APP_PICO_TRACKER_WSPR_2_USER_CONFIG_MANAGER_H__





















