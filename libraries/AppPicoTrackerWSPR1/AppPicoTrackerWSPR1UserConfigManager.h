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

    AppPicoTrackerWSPR1UserConfigManager(uint8_t pinConfigure)
    : pinConfigure_(pinConfigure)
    {
        // Nothing to do
    }
    
    uint8_t GetUserConfig(UserConfig &userConfig)
    {
        uint8_t retVal = 1;
        
        if (DetectUserWantsToConfigure())
        {
            retVal = InteractivelyConfigure(userConfig);
        }
        else
        {
            LogNNL(P("No configure signal -- "));
            
            retVal = ea_.Read(userConfig);
            
            if (retVal)
            {
                Log(P("OK: prior config found"));
            }
            else
            {
                Log(P("ERR: no prior config found"));
            }
        }
        
        return retVal;
    }
    
    uint8_t DetectUserWantsToConfigure()
    {
        uint8_t retVal = 0;
        
        // Force pin into a known high state.
        // The only way this pin could be low would be if something was pulling
        // it low.
        // That is our signal.
        PAL.PinMode(pinConfigure_, INPUT_PULLUP);
        
        Log(P("Ground to configure"));
        
        uint32_t timeNow = PAL.Millis();
        
        uint8_t cont = 1;
        while (cont)
        {
            if (PAL.DigitalRead(pinConfigure_) == 0)
            {
                retVal = 1;
                
                cont = 0;
            }
            else if (PAL.Millis() - timeNow >= 5000)
            {
                cont = 0;
            }
        }
        
        return retVal;
    }
    
    uint8_t InteractivelyConfigure(UserConfig &userConfig)
    {
        uint8_t retVal = 0;
        
        uint8_t userConfigAcquired = ea_.Read(userConfig);
        
        if (userConfigAcquired)
        {
            Log(P("Prior config loaded"));
        }
        else
        {
            Log(P("No prior config, defaulting"));
        }
        LogNL();
        
        SerialAsyncConsoleMenu<13, 1> menu;
        
        menu.RegisterParamSTR(P("id"),                           userConfig.device.id, UserConfig::ID_LEN);
        
        menu.RegisterParamSTR(P("callsign"),                     userConfig.wspr.callsign, UserConfig::CALLSIGN_LEN);
        
        menu.RegisterParamU8(P("solarMode"),                    &userConfig.power.solarMode);
        menu.RegisterParamU16(P("minMilliVoltGpsLocationLock"), &userConfig.power.minMilliVoltGpsLocationLock);
        menu.RegisterParamU16(P("minMilliVoltGpsTimeLock"),     &userConfig.power.minMilliVoltGpsTimeLock);
        menu.RegisterParamU16(P("minMilliVoltTransmit"),        &userConfig.power.minMilliVoltTransmit);
        
        menu.RegisterParamU32(P("gpsLockTimeoutMs"),            &userConfig.gps.gpsLockTimeoutMs);
        
        menu.RegisterParamU32(P("lhAltFtThreshold"),            &userConfig.geo.lowHighAltitudeFtThreshold);
        menu.RegisterParamU32(P("hAlt.wakeAndEvaluateMs"),      &userConfig.geo.highAltitude.wakeAndEvaluateMs);
        menu.RegisterParamU32(P("lAlt.wakeAndEvaluateMs"),      &userConfig.geo.lowAltitude.wakeAndEvaluateMs);
        menu.RegisterParamU32(P("lAlt.stickyMs"),               &userConfig.geo.lowAltitude.stickyMs);
        
        menu.RegisterParamI32(P("crystalCorrectionFactor"),     &userConfig.radio.mtCalibration.crystalCorrectionFactor);
        menu.RegisterParamI32(P("systemClockOffsetMs"),         &userConfig.radio.mtCalibration.systemClockOffsetMs);
        
        menu.RegisterCommand(P("run"), [](){
            Evm::GetInstance().EndMainLoop();
        });
        
        menu.SetOnSetCallback([this, &menu, &userConfig](){
            Log(P("Saving"));
            
            ea_.Write(userConfig);
            ea_.Read(userConfig);
            
            menu.ShowParams();
        });
        
        menu.ShowAll();
        menu.Start();
        Evm::GetInstance().HoldStackDangerously();
        
        return retVal;
    }
    
private:

    uint8_t pinConfigure_;
    
    EepromAccessor<UserConfig> ea_;

};














#endif  // __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__









