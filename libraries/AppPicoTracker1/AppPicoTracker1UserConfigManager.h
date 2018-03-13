#ifndef __APP_PICO_TRACKER_1_USER_CONFIG_MANAGER_H__
#define __APP_PICO_TRACKER_1_USER_CONFIG_MANAGER_H__


#include "PAL.h"
#include "Eeprom.h"
#include "Str.h"
#include "UtlSerial.h"


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
                uint32_t wakeAndEvaluateMs = 15L * 60L * 1000L;    // 15 min
            } highAltitude;
            
            struct
            {
                uint32_t wakeAndEvaluateMs =      60L * 1000L;     // 60 sec, 1 min
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
            Serial.print(F("No serial input -- "));
            
            retVal = ea_.Read(userConfig);
            
            if (retVal)
            {
                Serial.println(F("OK: prior config found"));
            }
            else
            {
                Serial.println(F("ERR: no prior config found"));
            }
        }

        return retVal;
    }


    
public:

    static uint8_t DetectSerialInput()
    {
        uint8_t retVal = 0;
        
        // Force pin, which could be floating, and is seen to float and cause,
        // false positives, into a known high state.
        // The only way this pin could be low would be if something was pulling
        // it low.
        // That is our signal.
        PAL.PinMode(PIN_SERIAL_RX, INPUT_PULLUP);
        
        if (PAL.DigitalRead(PIN_SERIAL_RX) == 0)
        {
            retVal = 1;
            
            Serial.println(F("Interactive serial config -- unground input to continue"));
            Serial.println();
            
            while (PAL.DigitalRead(PIN_SERIAL_RX) == 0) {}
        }
        
        // Restore pin back to normal mode
        PAL.PinMode(PIN_SERIAL_RX, INPUT);
        
        // Clear out any junk in the serial input stream from having to mess
        // around with signal transitions.
        // This was seen in testing.
        PAL.Delay(50);
        while (Serial.available())
        {
            Serial.read();
        }
        
        return retVal;
    }

    static uint8_t GetUserConfigBySerial(UserConfig &userConfig)
    {
        uint8_t userConfigAcquired = 0;
        
        userConfigAcquired = ea_.Read(userConfig);
        
        if (userConfigAcquired)
        {
            Serial.println(F("Prior config loaded"));
        }
        else
        {
            Serial.println(F("No prior config, loading defaults"));
        }
        
        uint8_t firstTime = 1;
        
        uint8_t cont = 1;
        while (cont)
        {
            const uint8_t BUF_SIZE = 100;
            char buf[BUF_SIZE];
            
            auto strDeviceId                         = PSTR("device.id");
            auto strAprsCallsign                     = PSTR("aprs.callsign");
            auto strRadioTransmitCount               = PSTR("radio.transmitCount");
            auto strRadioDelayMsBetweenTransmits     = PSTR("radio.delayMsBetweenTransmits");
            auto strGeoLowHighAltitudeFtThreshold    = PSTR("geo.lowHighAltitudeFtThreshold");
            auto strGeoHighAltitudeWakeAndEvaluateMs = PSTR("geo.highAltitude.wakeAndEvaluateMs");
            auto strGeoLowAltitudeWakeAndEvaluateMs  = PSTR("geo.lowAltitude.wakeAndEvaluateMs");
            auto strGeoDeadZoneWakeAndEvaluateMs     = PSTR("geo.deadZone.wakeAndEvaluateMs");
            
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
                        
                        Serial.println(F("Deleted EEPROM, restarting..."));
                        Serial.println();
                        PAL.Delay(100);
                        
                        PAL.SoftReset();
                    }
                    if (!strcmp_P(name, PSTR("run")))
                    {
                        cont = 0;
                        
                        understood = 1;
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
                    else if (!strcmp_P(name, strGeoDeadZoneWakeAndEvaluateMs))
                    {
                        userConfig.geo.deadZone.wakeAndEvaluateMs = atol(str.TokenAtIdx(1, ' '));
                        
                        understood = 1;
                        updated    = 1;
                    }
                }
                
                if (!understood)
                {
                    Serial.print(F("Not understood: "));
                    Serial.print('"');
                    Serial.print(str.UnsafePtr());
                    Serial.print('"');
                    Serial.println();
                }
                
                if (updated)
                {
                    Serial.println(F("Auto-saving changes."));
                    ea_.Write(userConfig);
                    
                    userConfigAcquired = 1;
                    
                    Serial.println();
                }
                
                if (updated || firstTime)
                {
                    #define TOF(x) ((const __FlashStringHelper *)(x))
                    Serial.print(TOF(strDeviceId));                         Serial.print('['); Serial.print(UserConfig::DEVICE_ID_LEN); Serial.print("] = "); Serial.println(userConfig.device.id);
                    Serial.print(TOF(strAprsCallsign));                     Serial.print('['); Serial.print(UserConfig::CALLSIGN_LEN); Serial.print("] = "); Serial.println(userConfig.aprs.callsign);
                    Serial.print(TOF(strRadioTransmitCount));               Serial.print(" = "); Serial.println(userConfig.radio.transmitCount);
                    Serial.print(TOF(strRadioDelayMsBetweenTransmits));     Serial.print(" = "); Serial.println(userConfig.radio.delayMsBetweenTransmits);
                    Serial.print(TOF(strGeoLowHighAltitudeFtThreshold));    Serial.print(" = "); Serial.println(userConfig.geo.lowHighAltitudeFtThreshold);
                    Serial.print(TOF(strGeoHighAltitudeWakeAndEvaluateMs)); Serial.print(" = "); Serial.println(userConfig.geo.highAltitude.wakeAndEvaluateMs);
                    Serial.print(TOF(strGeoLowAltitudeWakeAndEvaluateMs));  Serial.print(" = "); Serial.println(userConfig.geo.lowAltitude.wakeAndEvaluateMs);
                    Serial.print(TOF(strGeoDeadZoneWakeAndEvaluateMs));     Serial.print(" = "); Serial.println(userConfig.geo.deadZone.wakeAndEvaluateMs);

                    Serial.println();
                }
                
                if (firstTime)
                {
                    firstTime = 0;
                }
            }
        }
        
        return userConfigAcquired;
    }
    
    
    static EepromAccessor<UserConfig> ea_;
};


EepromAccessor<AppPicoTracker1UserConfigManager::UserConfig> AppPicoTracker1UserConfigManager::ea_;


#endif  // __APP_PICO_TRACKER_1_USER_CONFIG_MANAGER_H__









