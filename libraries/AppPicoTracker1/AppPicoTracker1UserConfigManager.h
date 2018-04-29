#ifndef __APP_PICO_TRACKER_1_USER_CONFIG_MANAGER_H__
#define __APP_PICO_TRACKER_1_USER_CONFIG_MANAGER_H__


#include "PAL.h"
#include "Eeprom.h"
#include "Str.h"
#include "Utl@fix@Serial.h"


class AppPicoTracker1UserConfigManager
{
private:
    static const uint8_t PIN_SERIAL_RX = 2;
    
public:

    struct UserConfig
    {
        static const uint8_t DEVICE_ID_LEN = 4;
        static const uint8_t CALLSIGN_LEN  = 6;
        
        UserConfig()
        {
            memset((void *)device.id,     '\0', DEVICE_ID_LEN + 1);
            memset((void *)aprs.callsign, '\0', CALLSIGN_LEN  + 1);
        }
        
        struct
        {
            char id[DEVICE_ID_LEN + 1];
        } device;
        
        struct
        {
            char callsign[CALLSIGN_LEN + 1];
        } aprs;
        
        struct
        {
            uint8_t  transmitCount           = 2;
            uint32_t delayMsBetweenTransmits = 3000;
        } radio;
        
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
            
            struct
            {
                uint32_t wakeAndEvaluateMs = 120L * 60L * 1000L;   // 120 min, 2 hr
            } deadZone;
        } geo;
    };
    
    
    
    static uint8_t GetUserConfig(UserConfig &userConfig)
    {
        uint8_t retVal = 0;
        
        // Check there is a signal on the serial-in line.
        // if yes, enter configuration mode.
        if (DetectSerialInput())
        {
            retVal = GetUserConfigBySerial(userConfig);
        }
        else
        {
            @fix@Serial.print(F("No serial input -- "));
            
            retVal = ea_.Read(userConfig);
            
            if (retVal)
            {
                @fix@Serial.println(F("OK: prior config found"));
            }
            else
            {
                @fix@Serial.println(F("ERR: no prior config found"));
            }
        }

        return retVal;
    }

    
public:

    static uint8_t DetectSerialInput()
    {
        uint8_t retVal = 0;
        
        @fix@Serial.println(F("Ground to configure"));
        PAL.Delay(5000);
        
        // Force pin, which could be floating, and is seen to float and cause,
        // false positives, into a known high state.
        // The only way this pin could be low would be if something was pulling
        // it low.
        // That is our signal.
        PAL.PinMode(PIN_SERIAL_RX, INPUT_PULLUP);
        
        if (PAL.DigitalRead(PIN_SERIAL_RX) == 0)
        {
            retVal = 1;
            
            @fix@Serial.println(F("OK: un-ground to continue\n"));
            
            while (PAL.DigitalRead(PIN_SERIAL_RX) == 0) {}
        }
        
        // Restore pin back to normal mode
        PAL.PinMode(PIN_SERIAL_RX, INPUT);
        
        // Clear out any junk in the serial input stream from having to mess
        // around with signal transitions.
        // This was seen in testing.
        PAL.Delay(50);
        while (@fix@Serial.available())
        {
            @fix@Serial.read();
        }
        
        return retVal;
    }

    static uint8_t GetUserConfigBySerial(UserConfig &userConfig)
    {
        uint8_t userConfigAcquired = 0;
        
        userConfigAcquired = ea_.Read(userConfig);
        
        if (userConfigAcquired)
        {
            @fix@Serial.println(F("Prior config loaded"));
        }
        else
        {
            @fix@Serial.println(F("No prior config, defaulting"));
        }
        
        uint8_t firstTime = 1;
        
        uint8_t cont = 1;
        while (cont)
        {
            const uint8_t BUF_SIZE = 100;
            char buf[BUF_SIZE];
            
            auto strDeviceId                         = PSTR("id");
            auto strAprsCallsign                     = PSTR("callsign");
            auto strRadioTransmitCount               = PSTR("transmitCount");
            auto strRadioDelayMsBetweenTransmits     = PSTR("msBetweenTransmits");
            auto strGeoLowHighAltitudeFtThreshold    = PSTR("lhAltFtThreshold");
            auto strGeoHighAltitudeWakeAndEvaluateMs = PSTR("hAlt.wakeAndEvaluateMs");
            auto strGeoLowAltitudeWakeAndEvaluateMs  = PSTR("lAlt.wakeAndEvaluateMs");
            auto strGeoLowAltitudeStickyMs           = PSTR("lAlt.stickyMs");
            auto strGeoDeadZoneWakeAndEvaluateMs     = PSTR("dz.wakeAndEvaluateMs");
            
            uint8_t strLen = 0;
            if (!firstTime)
            {
                strLen = SerialReadLine(buf, BUF_SIZE);
            }
            
            if (strLen || firstTime)
            {
                Str str(buf);
                
                uint8_t tokenCount = str.TokenCount(' ');
                
                uint8_t understood  = firstTime;
                uint8_t updated     = 0;
                
                if (tokenCount == 1)
                {
                    const char *name = str.TokenAtIdx(0, ' ');
                    
                    if (!strcmp_P(name, PSTR("reset")))
                    {
                        UserConfig userConfigTmp;
                        
                        ea_.Write(userConfigTmp);
                        ea_.Read(userConfig);
                        
                        understood = 1;
                        updated    = 1;
                    }
                    if (!strcmp_P(name, PSTR("delete")))
                    {
                        // Debug only
                        ea_.Delete();
                        
                        @fix@Serial.println(F("Deleted"));
                        @fix@Serial.println();
                        PAL.Delay(100);
                        
                        PAL.SoftReset();
                    }
                    if (!strcmp_P(name, PSTR("run")))
                    {
                        cont = 0;
                        
                        understood = 1;
                        updated = 1;
                    }
                }
                else if (tokenCount >= 2)
                {
                    const char *name = str.TokenAtIdx(0, ' ');
                    
                    if (!strcmp_P(name, strDeviceId))
                    {
                        const char *val = str.TokenAtIdx(1, ' ');
                        
                        memset((void *)userConfig.device.id, '\0', UserConfig::DEVICE_ID_LEN + 1);
                        strncpy(userConfig.device.id, val, UserConfig::DEVICE_ID_LEN + 1);
                        userConfig.device.id[UserConfig::DEVICE_ID_LEN] = '\0';
                        
                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, strAprsCallsign))
                    {
                        const char *val = str.TokenAtIdx(1, ' ');
                        
                        memset((void *)userConfig.aprs.callsign, '\0', UserConfig::CALLSIGN_LEN + 1);
                        strncpy(userConfig.aprs.callsign, val, UserConfig::CALLSIGN_LEN + 1);
                        userConfig.aprs.callsign[UserConfig::CALLSIGN_LEN] = '\0';

                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, strRadioTransmitCount))
                    {
                        userConfig.radio.transmitCount = atoi(str.TokenAtIdx(1, ' '));
                        
                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, strRadioDelayMsBetweenTransmits))
                    {
                        userConfig.radio.delayMsBetweenTransmits = atol(str.TokenAtIdx(1, ' '));
                        
                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, strGeoLowHighAltitudeFtThreshold))
                    {
                        userConfig.geo.lowHighAltitudeFtThreshold = atol(str.TokenAtIdx(1, ' '));
                        
                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, strGeoHighAltitudeWakeAndEvaluateMs))
                    {
                        userConfig.geo.highAltitude.wakeAndEvaluateMs = atol(str.TokenAtIdx(1, ' '));
                        
                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, strGeoLowAltitudeWakeAndEvaluateMs))
                    {
                        userConfig.geo.lowAltitude.wakeAndEvaluateMs = atol(str.TokenAtIdx(1, ' '));
                        
                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, strGeoLowAltitudeStickyMs))
                    {
                        userConfig.geo.lowAltitude.stickyMs = atol(str.TokenAtIdx(1, ' '));
                        
                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, strGeoDeadZoneWakeAndEvaluateMs))
                    {
                        userConfig.geo.deadZone.wakeAndEvaluateMs = atol(str.TokenAtIdx(1, ' '));
                        
                        understood = 1;
                        updated    = 1;
                    }
                }
                
                if (!understood)
                {
                    @fix@Serial.print(F("ERR: "));
                    @fix@Serial.print('"');
                    @fix@Serial.print(str.UnsafePtr());
                    @fix@Serial.println('"');
                }
                
                if (updated)
                {
                    @fix@Serial.println(F("\nSaved"));
                    ea_.Write(userConfig);
                    
                    userConfigAcquired = 1;
                    
                    @fix@Serial.println();
                }
                
                if (updated || firstTime)
                {
                    #define TOF(x) ((const __FlashStringHelper *)(x))
                    @fix@Serial.print(TOF(strDeviceId));                         @fix@Serial.print('['); @fix@Serial.print(UserConfig::DEVICE_ID_LEN); @fix@Serial.print("] = "); @fix@Serial.println(userConfig.device.id);
                    @fix@Serial.print(TOF(strAprsCallsign));                     @fix@Serial.print('['); @fix@Serial.print(UserConfig::CALLSIGN_LEN); @fix@Serial.print("] = "); @fix@Serial.println(userConfig.aprs.callsign);
                    @fix@Serial.print(TOF(strRadioTransmitCount));               @fix@Serial.print(" = "); @fix@Serial.println(userConfig.radio.transmitCount);
                    PrintNameAndMs(strRadioDelayMsBetweenTransmits, userConfig.radio.delayMsBetweenTransmits);
                    @fix@Serial.print(TOF(strGeoLowHighAltitudeFtThreshold));    @fix@Serial.print(" = "); @fix@Serial.println(userConfig.geo.lowHighAltitudeFtThreshold);
                    PrintNameAndMs(strGeoHighAltitudeWakeAndEvaluateMs, userConfig.geo.highAltitude.wakeAndEvaluateMs);
                    PrintNameAndMs(strGeoLowAltitudeWakeAndEvaluateMs, userConfig.geo.lowAltitude.wakeAndEvaluateMs);
                    PrintNameAndMs(strGeoLowAltitudeStickyMs, userConfig.geo.lowAltitude.stickyMs);
                    PrintNameAndMs(strGeoDeadZoneWakeAndEvaluateMs, userConfig.geo.deadZone.wakeAndEvaluateMs);
                    
                    @fix@Serial.println();
                }
                
                if (firstTime)
                {
                    firstTime = 0;
                }
            }
        }
        
        return userConfigAcquired;
    }
    
    static void PrintNameAndMs(const char *name, uint32_t valueMs)
    {
        uint32_t tmp = valueMs;
        
        @fix@Serial.print(TOF(name));
        @fix@Serial.print(F(" = "));
        @fix@Serial.print(valueMs);
        
        tmp = tmp / 1000;
        @fix@Serial.print(F(" ("));
        @fix@Serial.print(tmp);
        @fix@Serial.print(F("s, "));
        
        tmp = tmp / 60;
        @fix@Serial.print(tmp);
        @fix@Serial.print(F("m, "));
        
        tmp = tmp / 60;
        @fix@Serial.print(tmp);
        @fix@Serial.println(F("h)"));
    }
    
    
    static EepromAccessor<UserConfig> ea_;
};


EepromAccessor<AppPicoTracker1UserConfigManager::UserConfig> AppPicoTracker1UserConfigManager::ea_;


#endif  // __APP_PICO_TRACKER_1_USER_CONFIG_MANAGER_H__









