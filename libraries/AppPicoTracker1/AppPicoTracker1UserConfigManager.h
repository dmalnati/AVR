#ifndef __APP_PICO_TRACKER_1_USER_CONFIG_MANAGER_H__
#define __APP_PICO_TRACKER_1_USER_CONFIG_MANAGER_H__


#include "Eeprom.h"
#include "Str.h"
#include "UtlSerial.h"


class AppPicoTracker1UserConfigManager
{
public:
    struct UserConfig
    {
    public:
        static const uint8_t DEVICE_ID_LEN     = 20;
        static const uint8_t CALLSIGN_LEN      = 6;
        static const uint8_t NOTES_MESSAGE_LEN = 10;
        
    public:
        UserConfig()
        {
            memset((void *)deviceId,     '\0', DEVICE_ID_LEN     + 1);
            memset((void *)callsign,     '\0', CALLSIGN_LEN      + 1);
            memset((void *)notesMessage, '\0', NOTES_MESSAGE_LEN + 1);
        }
        
        char *GetDeviceId() { return deviceId; }
        void SetDeviceId(const char *val) { strncpy(deviceId, val, DEVICE_ID_LEN + 1); deviceId[DEVICE_ID_LEN] = '\0'; }
        
        char *GetCallsign() { return callsign; }
        void SetCallsign(const char *val) { strncpy(callsign, val, CALLSIGN_LEN + 1); callsign[CALLSIGN_LEN] = '\0'; }
        
        uint32_t GetReportIntervalSecs() { return reportIntervalSecs; }
        void SetReportIntervalSecs(uint32_t val) { reportIntervalSecs = val; }
        
        uint8_t GetTransmitCount() { return transmitCount; }
        void SetTransmitCount(uint8_t val) { transmitCount = val; }
        
        uint32_t GetDelaySecsBetweenTransmits() { return delaySecsBetweenTransmits; }
        void SetDelaySecsBetweenTransmits(uint32_t val) { delaySecsBetweenTransmits = val; }

        char *GetNotesMessage() { return notesMessage; }
        void SetNotesMessage(const char *val) { strncpy(notesMessage, val, NOTES_MESSAGE_LEN + 1); notesMessage[NOTES_MESSAGE_LEN] = '\0'; }
        
        void Print()
        {
            Serial.println(F("Current Configuration"));
            Serial.println(F("---------------------"));
            
            Serial.println(F("Device Unique Identification"));
            
            Serial.print(F("    deviceId["));
            Serial.print(DEVICE_ID_LEN);
            Serial.print(F("]: "));
            Serial.print(GetDeviceId());
            Serial.println();
            Serial.println();
            
            
            Serial.println(F("Your Identity"));
            
            Serial.print(F("    callsign["));
            Serial.print(CALLSIGN_LEN);
            Serial.print(F("]: "));
            Serial.print(GetCallsign());
            Serial.println();
            Serial.println();
            
            
            Serial.println(F("Reporting Interval"));
            
            Serial.print(F("    reportIntervalSecs: "));
            Serial.print(GetReportIntervalSecs());
            Serial.println();
            Serial.println();
            
            
            Serial.println(F("Reliability of Transmission"));
            
            Serial.print(F("    transmitCount: "));
            Serial.print(GetTransmitCount());
            Serial.println();
            
            Serial.print(F("    delaySecsBetweenTransmits: "));
            Serial.print(GetDelaySecsBetweenTransmits());
            Serial.println();
            Serial.println();
            
            
            Serial.println(F("Message Content"));
            Serial.print(F("    notesMessage["));
            Serial.print(NOTES_MESSAGE_LEN);
            Serial.print(F("]: "));
            Serial.print(GetNotesMessage());
            Serial.println();
            
            Serial.println(F("---------------------"));
        }
        
    private:
        char deviceId[DEVICE_ID_LEN + 1];
        
        char callsign[CALLSIGN_LEN + 1];
        
        uint32_t reportIntervalSecs = 60;
        
        uint8_t  transmitCount             = 2;
        uint32_t delaySecsBetweenTransmits = 3;
        
        char notesMessage[NOTES_MESSAGE_LEN + 1];
    };

    
public:

    static uint8_t GetUserConfig(UserConfig &userConfig)
    {
        uint8_t retVal = 0;
        
        uint8_t configurationAvailableToValidate = 0;
        
        // Check serial in, if it's high, we assume a serial-speaker is on the
        // other end.

        // Was getting inconsistent readings here on startup.
        // Wait briefly for pin state to settle.
        PAL.Delay(20);
        
        if (PAL.DigitalRead(2))
        {
            Serial.println(F("Interactive configuration mode enabled."));
            
            configurationAvailableToValidate = GetUserConfigBySerial(userConfig);
        }
        else
        {
            Serial.println(F("No Serial-In detected, attempting to use previously-stored configuration."));
            
            EepromAccessor<UserConfig> ea;
            if (ea.Read(userConfig))
            {
                Serial.println(F("OK: Previously-stored configuration found and loaded."));

                configurationAvailableToValidate = 1;
            }
            else
            {
                Serial.println(F("ERR: No stored configuration found."));
            }
        }
        
        if (configurationAvailableToValidate)
        {
            // Check configuration
            uint8_t errCount = 0;
            
            Serial.println(F("Validating configuration."));
            
            if (!strlen(userConfig.GetDeviceId()))
            {
                Serial.println(F("ERR: deviceId blank."));
                
                ++errCount;
            }
            
            if (!strlen(userConfig.GetCallsign()))
            {
                Serial.println(F("ERR: callsign blank."));
                
                ++errCount;
            }
            
            if (userConfig.GetReportIntervalSecs() == 0)
            {
                Serial.println(F("ERR: reportIntervalSecs can't be zero."));
                
                ++errCount;
            }
            
            if (userConfig.GetTransmitCount() == 0)
            {
                Serial.println(F("ERR: transmitCount can't be zero."));
                
                ++errCount;
            }
            
            if (!errCount)
            {
                retVal = 1;
                
                Serial.println(F("OK: Configuration valid."));
            }
            else
            {
                Serial.print(F("ERR: "));
                Serial.print(errCount);
                Serial.println(F(" configuration errors detected."));
            }
        }
        else
        {
            Serial.println(F("ERR: No stored configuration found, no new configuration set."));
        }
        
        return retVal;
    }
    
private:
    static uint8_t GetUserConfigBySerial(UserConfig &userConfig)
    {
        uint8_t userConfigAcquired = 0;
        
        EepromAccessor<UserConfig> ea;
        userConfigAcquired = ea.Read(userConfig);
        
        if (userConfigAcquired)
        {
            Serial.println(F("Previously-stored configuration found and loaded."));
        }
        else
        {
            Serial.println(F("No Previously-stored configuration found, loading default values."));
        }
        
        PrintHelp(userConfig);
        
        uint8_t cont = 1;
        while (cont)
        {
            const uint8_t BUF_SIZE = 100;
            char buf[BUF_SIZE];
        
            uint8_t strLen = SerialReadLine(buf, BUF_SIZE);
            
            if (strLen)
            {
                Str str(buf);
                
                uint8_t tokenCount = str.TokenCount(' ');
                
                uint8_t understood  = 0;
                uint8_t updated     = 0;
                
                if (!strcmp_P(str.UnsafePtr(), PSTR("help")) ||
                    !strcmp_P(str.UnsafePtr(), PSTR("?")))
                {
                    PrintHelp(userConfig);
                    
                    understood = 1;
                }
                else if (tokenCount == 1)
                {
                    const char *name = str.TokenAtIdx(0, ' ');
                    
                    if (!strcmp_P(name, PSTR("reset")))
                    {
                        UserConfig userConfigTmp;
                        
                        ea.Write(userConfigTmp);
                        ea.Read(userConfig);
                        
                        understood = 1;
                        updated    = 1;
                    }
                    if (!strcmp_P(name, PSTR("delete")))
                    {
                        // Debug only
                        ea.Delete();
                        
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
                    
                    if (!strcmp_P(name, PSTR("deviceId")))
                    {
                        userConfig.SetDeviceId(str.UnsafePtrAtTokenAtIdx(1, ' '));
                        
                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, PSTR("callsign")))
                    {
                        userConfig.SetCallsign(str.TokenAtIdx(1, ' '));
                        
                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, PSTR("reportIntervalSecs")))
                    {
                        userConfig.SetReportIntervalSecs(atol(str.TokenAtIdx(1, ' ')));
                        
                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, PSTR("transmitCount")))
                    {
                        userConfig.SetTransmitCount(atoi(str.TokenAtIdx(1, ' ')));
                        
                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, PSTR("delaySecsBetweenTransmits")))
                    {
                        userConfig.SetDelaySecsBetweenTransmits(atol(str.TokenAtIdx(1, ' ')));
                        
                        understood = 1;
                        updated    = 1;
                    }
                    else if (!strcmp_P(name, PSTR("notesMessage")))
                    {
                        userConfig.SetNotesMessage(str.UnsafePtrAtTokenAtIdx(1, ' '));
                        
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
                    ea.Write(userConfig);
                    
                    userConfigAcquired = 1;
                    
                    Serial.println();
                    userConfig.Print();
                    Serial.println();
                }
            }
        }
        
        return userConfigAcquired;
    }
    
    static void PrintHelp(UserConfig &userConfig)
    {
        Serial.println();
        
        userConfig.Print();
        
        Serial.println();
        Serial.println(F("To set any parameter, type its name, space, and a new value"));
        Serial.println(F("Then hit enter."));
        Serial.println(F("Any changes are saved automatically."));
        Serial.println();
        
        Serial.println(F("To start over, type reset and hit enter."));
        Serial.println();
        
        Serial.println(F("When finished, you can either:"));
        Serial.println(F("- disconnect the Serial-In line and restart the device -or-"));
        Serial.println(F("- type run and hit enter"));
        Serial.println();
        
        Serial.print(F("Type help or ? any time."));
        Serial.println();
        
        Serial.println();
    }
};


#endif  // __APP_PICO_TRACKER_1_USER_CONFIG_MANAGER_H__









