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
        
        char *callsign[CALLSIGN_LEN + 1] = { 0 };
        
        WSPRMessageTransmitter::Calibration mtCalibration;
    };
    
    static uint8_t GetUserConfig(uint8_t pinConfigure, UserConfig &userConfig)
    {
        uint8_t retVal = 1;
        
        EepromAccessor<UserConfig> ea;

        if (DetectSerialInput(pinConfigure))
        {
            retVal = GetUserConfigBySerial(userConfig, ea);
        }
        else
        {
            LogNNL(P("No configure signal -- "));
            
            retVal = ea.Read(userConfig);
            
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
    
    static uint8_t DetectSerialInput(uint8_t pinConfigure)
    {
        uint8_t retVal = 0;
        
        // Force pin into a known high state.
        // The only way this pin could be low would be if something was pulling
        // it low.
        // That is our signal.
        PAL.PinMode(pinConfigure, INPUT_PULLUP);
        
        Log(P("Ground to configure"));
        PAL.Delay(5000);
        
        if (PAL.DigitalRead(pinConfigure) == 0)
        {
            retVal = 1;
            
            Log(P("OK: un-ground to continue\n"));
            
            while (PAL.DigitalRead(pinConfigure) == 0) {}
        }
        
        return retVal;
    }
    
    static uint8_t GetUserConfigBySerial(UserConfig &userConfig, EepromAccessor<UserConfig> &ea)
    {
        uint8_t retVal = 0;
        
        uint8_t userConfigAcquired = ea.Read(userConfig);
        
        if (userConfigAcquired)
        {
            Log(P("Prior config loaded"));
        }
        else
        {
            Log(P("No prior config, defaulting"));
        }
        
        SerialAsyncConsole<5,20>  console;
        
        console.RegisterCommand("set", [](char *cmdStr){
            Str str(cmdStr);
            
            uint8_t handled = 1;
            
            uint8_t tokenCount = str.TokenCount(' ');
            if (tokenCount >= 2)
            {
                const char *p = str.TokenAtIdx(1, ' ');
                
                if (tokenCount == 3)
                {
                    if (!strcmp(p, "minMilliVoltGpsLocationLock"))
                    {
                    }
                    else if (!strcmp(p, "minMilliVoltGpsTimeLock"))
                    {
                    }
                    else if (!strcmp(p, "minMilliVoltTransmit"))
                    {
                    }
                    else if (!strcmp(p, "gpsLockTimeoutMs"))
                    {
                    }
                    else if (!strcmp(p, "callsign"))
                    {
                    }
                    else if (!strcmp(p, "crystalCorrectionFactor "))
                    {
                    }
                    else if (!strcmp(p, "systemClockOffsetMs "))
                    {
                    }
                    else
                    {
                        handled = 0;
                    }
                }
                else if (tokenCount == 2)
                {
                    if (!strcmp(p, "run"))
                    {
                        Evm::GetInstance().EndMainLoop();
                    }
                    else
                    {
                        handled = 0;
                    }
                }
            }
            
            if (handled)
            {
                // store updated config in EEPROM
            }
        });
        
        console.Start();
        Evm::GetInstance().HoldStackDangerously();
        
        return retVal;
    }

};


#endif  // __APP_PICO_TRACKER_WSPR_1_USER_CONFIG_MANAGER_H__









