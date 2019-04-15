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
        // Solar vs battery related
        uint8_t solarMode = 0;
        
        uint16_t minMilliVoltGpsLocationLock = 0;
        uint16_t minMilliVoltGpsTimeLock     = 0;
        uint16_t minMilliVoltTransmit        = 0;
        
        // GPS stuff
        uint32_t gpsLockTimeoutMs = 70000;
        
        // WSPR Stuff
        static const uint8_t CALLSIGN_LEN = 6;
        
        char callsign[CALLSIGN_LEN + 1] = { 0 };
        
        WSPRMessageTransmitter::Calibration mtCalibration;
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
        
        SerialAsyncConsoleMenu<8, 1> menu;
        
        menu.RegisterParamU8(P("solarMode"),                    &userConfig.solarMode);
        menu.RegisterParamU16(P("minMilliVoltGpsLocationLock"), &userConfig.minMilliVoltGpsLocationLock);
        menu.RegisterParamU16(P("minMilliVoltGpsTimeLock"),     &userConfig.minMilliVoltGpsTimeLock);
        menu.RegisterParamU16(P("minMilliVoltTransmit"),        &userConfig.minMilliVoltTransmit);
        menu.RegisterParamU32(P("gpsLockTimeoutMs"),            &userConfig.gpsLockTimeoutMs);
        menu.RegisterParamSTR(P("callsign"),                     userConfig.callsign, UserConfig::CALLSIGN_LEN);
        menu.RegisterParamI32(P("crystalCorrectionFactor"),     &userConfig.mtCalibration.crystalCorrectionFactor);
        menu.RegisterParamI32(P("systemClockOffsetMs"),         &userConfig.mtCalibration.systemClockOffsetMs);
        
        menu.RegisterCommand(P("run"), [](){
            Evm::GetInstance().EndMainLoop();
        });
        
        menu.SetOnSetCallback([this, &menu, &userConfig](){
            Log(P("Saving"));
            
            ea_.Write(userConfig);
            
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









