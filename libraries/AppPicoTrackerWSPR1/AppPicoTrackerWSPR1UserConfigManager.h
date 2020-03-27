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
            char buf[StrFormat::HHMMSSMMM_BUF_SIZE_NEEDED] = { 0 };
            
            StrFormat::DurationMsToHHMMSSMMM(*(uint32_t *)param.paramPtr, buf);
            StrFormat::TrimMsFromHHMMSSMMM(buf);
            StrFormat::TrimLeadingPaddingFromHHMMSSMMM(buf);
            
            LogNNL(buf);
        });
    }

    virtual void OnDone() override
    {
        // It's the percent speed the clock is running at.
        // If 1.00, clock is running at actual time (100%).
        // If < 1.00, eg 0.97, clock is running at 97% speed (so it's slow).
        // If > 1.00, eg 1.03, clock is running at 103% speed (so it's fast).

        // Since we now have in hand the systemClockOffsetMs, and we know it's
        // in terms of a 683ms signal, we can calculate the speed of the device.
        //
        // A negative value of systemClockOffsetMs means the device is running
        // too slowly.
        // 
        // By example, if systemClockOffsetMs is -20ms, then (638+-20)/683=0.97.

        const uint32_t PULSE_DURATION_MS = 683;

        Config().calculated.runningAtSpeedFactor = 
            (double)(PULSE_DURATION_MS + Config().radio.mtCalibration.systemClockOffsetMs) / PULSE_DURATION_MS;
    }
    

private:

    void SetupMenu()
    {
        Menu().RegisterParamSTR(P("trackerId"),                    Config().device.id, AppPicoTrackerWSPR1UserConfig::ID_LEN);
        
        Menu().RegisterParamSTR(P("wsprCallsignId"),               Config().wspr.callsignId, AppPicoTrackerWSPR1UserConfig::CALLSIGN_ID_LEN);
        
        //Menu().RegisterParamU8(P("solarMode"),                    &Config().power.solarMode);
        //Menu().RegisterParamU16(P("minMilliVoltGpsLocationLock"), &Config().power.minMilliVoltGpsLocationLock);
        //Menu().RegisterParamU16(P("minMilliVoltGpsTimeLock"),     &Config().power.minMilliVoltGpsTimeLock);
        //Menu().RegisterParamU16(P("minMilliVoltTransmit"),        &Config().power.minMilliVoltTransmit);
        
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





















