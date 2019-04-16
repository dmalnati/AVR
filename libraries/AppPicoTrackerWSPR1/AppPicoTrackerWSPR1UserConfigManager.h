#ifndef __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__
#define __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__


#include "WSPRMessageTransmitter.h"
#include "Eeprom.h"
#include "SerialInput.h"



class AppPicoTrackerWSPR1UserConfigManager
{
public:

    struct UserConfig
    {
        static const uint8_t ID_LEN       = 4;
        static const uint8_t CALLSIGN_LEN = 6;
        
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
            uint32_t gpsLockTimeoutMs = 70000;
        } gps;
        
        // WSPR Stuff
        struct
        {
            char callsign[CALLSIGN_LEN + 1] = { 0 };
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

    
public:

    AppPicoTrackerWSPR1UserConfigManager(uint8_t pinConfigure, UserConfig &userConfig)
    : pinConfigure_(pinConfigure)
    , userConfig_(userConfig)
    , saved_(0)
    {
        // Nothing to do
    }
    
    uint8_t GetUserConfig()
    {
        uint8_t retVal = 1;
        
        SetupMenu();
        
        if (DetectUserWantsToConfigure())
        {
            retVal = InteractivelyConfigure();
        }
        else
        {
            LogNNL(P("No configure signal -- "));
            
            retVal = ea_.Read(userConfig_);
            
            if (retVal)
            {
                Log(P("OK: prior config found"));
                
                menu_.ShowParams();
                LogNL();
            }
            else
            {
                Log(P("ERR: no prior config found"));
            }
        }
        
        return retVal;
    }
    
    
private:

    uint8_t DetectUserWantsToConfigure()
    {
        uint8_t retVal = 0;
        
        // Force pin into a known high state.
        // The only way this pin could be low would be if something was pulling
        // it low.
        // That is our signal.
        PAL.PinMode(pinConfigure_, INPUT_PULLUP);
        
        LogNNL(P("Ground to configure "));
        
        uint32_t timeStart   = PAL.Millis();
        uint32_t timeLastDot = timeStart;
        
        uint8_t cont = 1;
        while (cont)
        {
            uint32_t timeNow = PAL.Millis();
            
            if (timeNow - timeLastDot >= 1000)
            {
                timeLastDot = timeLastDot + 1000;
                
                LogNNL('.');
            }
            
            if (PAL.DigitalRead(pinConfigure_) == 0)
            {
                retVal = 1;
                
                cont = 0;
            }
            else if (timeNow - timeStart >= 5000)
            {
                cont = 0;
            }
        }
        LogNL();
        
        return retVal;
    }
    
    uint8_t InteractivelyConfigure()
    {
        uint8_t retVal = 0;
        
        uint8_t userConfigAcquired = ea_.Read(userConfig_);
        
        if (userConfigAcquired)
        {
            Log(P("Prior config loaded"));
        }
        else
        {
            Log(P("No prior config, defaulting"));
        }
        LogNL();
        
        saved_ = 0;
        
        menu_.ShowAll();
        menu_.Start();
        Evm::GetInstance().HoldStackDangerously();
        
        retVal = saved_;
        
        return retVal;
    }
    
    void SetupMenu()
    {
        menu_.RegisterParamSTR(P("id"),                           userConfig_.device.id, UserConfig::ID_LEN);
        
        menu_.RegisterParamSTR(P("callsign"),                     userConfig_.wspr.callsign, UserConfig::CALLSIGN_LEN);
        
        menu_.RegisterParamU8(P("solarMode"),                    &userConfig_.power.solarMode);
        menu_.RegisterParamU16(P("minMilliVoltGpsLocationLock"), &userConfig_.power.minMilliVoltGpsLocationLock);
        menu_.RegisterParamU16(P("minMilliVoltGpsTimeLock"),     &userConfig_.power.minMilliVoltGpsTimeLock);
        menu_.RegisterParamU16(P("minMilliVoltTransmit"),        &userConfig_.power.minMilliVoltTransmit);
        
        menu_.RegisterParamU32(P("gpsLockTimeoutMs"),            &userConfig_.gps.gpsLockTimeoutMs);
        
        menu_.RegisterParamU32(P("lhAltFtThreshold"),            &userConfig_.geo.lowHighAltitudeFtThreshold);
        menu_.RegisterParamU32(P("hAlt.wakeAndEvaluateMs"),      &userConfig_.geo.highAltitude.wakeAndEvaluateMs);
        menu_.RegisterParamU32(P("lAlt.wakeAndEvaluateMs"),      &userConfig_.geo.lowAltitude.wakeAndEvaluateMs);
        menu_.RegisterParamU32(P("lAlt.stickyMs"),               &userConfig_.geo.lowAltitude.stickyMs);
        
        menu_.RegisterParamI32(P("crystalCorrectionFactor"),     &userConfig_.radio.mtCalibration.crystalCorrectionFactor);
        menu_.RegisterParamI32(P("systemClockOffsetMs"),         &userConfig_.radio.mtCalibration.systemClockOffsetMs);
        
        menu_.RegisterCommand(P("run"), [](){
            Evm::GetInstance().EndMainLoop();
        });
        
        menu_.SetOnSetCallback([this](){
            LogNL();
            Log(P("Saving"));
            LogNL();
            
            ea_.Write(userConfig_);
            ea_.Read(userConfig_);
            
            saved_ = 1;
            
            menu_.ShowParams();
        });
    }
    
private:

    uint8_t pinConfigure_;
    
    UserConfig &userConfig_;
    
    EepromAccessor<UserConfig> ea_;
    
    SerialAsyncConsoleMenu<13, 1> menu_;
    uint8_t saved_;

};














#endif  // __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__









