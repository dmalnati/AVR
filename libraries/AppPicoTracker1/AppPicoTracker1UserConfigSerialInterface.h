#ifndef __APP_PICO_TRACKER_1_USER_CONFIG_SERIAL_INTERFACE_H__
#define __APP_PICO_TRACKER_1_USER_CONFIG_SERIAL_INTERFACE_H__


#include "Eeprom.h"
#include "Str.h"
#include "UtlSerial.h"


class AppPicoTracker1UserConfigSerialInterface
{
public:

    void GetUserConfig()
    {
        EepromAccessor<UserConfig> ea;
        ea.Read(userConfig_);
        
        PrintHelp();
        
        const uint8_t BUF_SIZE = 100;
        char buf[BUF_SIZE];
        
        
        while (1)
        {
            uint8_t strLen = SerialReadLine(buf, BUF_SIZE);
            
            if (strLen)
            {
                Str str(buf);
                
                uint8_t tokenCount = str.TokenCount(' ');
                
                uint8_t understood  = 1;
                uint8_t updated     = 1;
                
                if (!strcmp_P(str.UnsafePtr(), PSTR("help")) ||
                    !strcmp_P(str.UnsafePtr(), PSTR("?")))
                {
                    PrintHelp();
                    
                    updated = 0;
                }
                else if (tokenCount == 1)
                {
                    const char *name = str.TokenAtIdx(0, ' ');
                    
                    if (!strcmp_P(name, PSTR("reset")))
                    {
                        UserConfig userConfigTmp;
                        
                        ea.Write(userConfigTmp);
                        ea.Read(userConfig_);
                    }
                    else
                    {
                        understood  = 0;
                        updated     = 0;
                    }
                }
                else if (tokenCount >= 2)
                {
                    const char *name = str.TokenAtIdx(0, ' ');
                    
                    if (!strcmp_P(name, PSTR("deviceId")))
                    {
                        userConfig_.SetDeviceId(str.UnsafePtrAtTokenAtIdx(1, ' '));
                    }
                    else if (!strcmp_P(name, PSTR("callsign")))
                    {
                        userConfig_.SetCallsign(str.TokenAtIdx(1, ' '));
                    }
                    else if (!strcmp_P(name, PSTR("ssid")))
                    {
                        userConfig_.SetSsid(atoi(str.TokenAtIdx(1, ' ')));
                    }
                    else if (!strcmp_P(name, PSTR("reportIntervalSecs")))
                    {
                        userConfig_.SetReportIntervalSecs(atol(str.TokenAtIdx(1, ' ')));
                    }
                    else if (!strcmp_P(name, PSTR("transmitCount")))
                    {
                        userConfig_.SetTransmitCount(atoi(str.TokenAtIdx(1, ' ')));
                    }
                    else if (!strcmp_P(name, PSTR("delaySecsBetweenTransmits")))
                    {
                        userConfig_.SetDelaySecsBetweenTransmits(atol(str.TokenAtIdx(1, ' ')));
                    }
                    else if (!strcmp_P(name, PSTR("notesMessage")))
                    {
                        userConfig_.SetNotesMessage(str.UnsafePtrAtTokenAtIdx(1, ' '));
                    }
                    else
                    {
                        understood  = 0;
                        updated     = 0;
                    }
                }
                else
                {
                    understood = 0;
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
                    ea.Write(userConfig_);
                    
                    Serial.println();
                    PrintCurrentState();
                    Serial.println();
                }
            }
        }
    }

    
    
    
    
private:
    
    void PrintHelp()
    {
        Serial.println();
        
        PrintCurrentState();
        
        Serial.println();
        Serial.print(F("To override any parameter, type its name, space, and "));
        Serial.print(F("a new value."));
        Serial.println();
        Serial.println(F("Then hit enter."));
        
        Serial.println(F("Any changes are saved automatically."));
        Serial.println();
        Serial.println(F("To start over, type reset and hit enter."));
        Serial.println();
        
        Serial.print(F("When finished, disconnect the Serial-In line "));
        Serial.print(F("and then restart the device."));
        Serial.println();
        Serial.println();
        
        Serial.print(F("Type help or ? any time."));
        Serial.println();
        
        Serial.println();
    }
    


    void PrintCurrentState()
    {
        Serial.println(F("Current Configuration"));
        Serial.println(F("---------------------"));
        
        Serial.println(F("Device Unique Identification"));
        
        Serial.print(F("    deviceId["));
        Serial.print(UserConfig::DEVICE_ID_LEN);
        Serial.print(F("]: "));
        Serial.print(userConfig_.GetDeviceId());
        Serial.println();
        Serial.println();
        
        
        Serial.println(F("Your Identity"));
        
        Serial.print(F("    callsign["));
        Serial.print(UserConfig::CALLSIGN_LEN);
        Serial.print(F("]: "));
        Serial.print(userConfig_.GetCallsign());
        Serial.println();
        
        Serial.print(F("    ssid: "));
        Serial.print(userConfig_.GetSsid());
        Serial.println();
        Serial.println();
        
        
        Serial.println(F("Reporting Interval"));
        
        Serial.print(F("    reportIntervalSecs: "));
        Serial.print(userConfig_.GetReportIntervalSecs());
        Serial.println();
        Serial.println();
        
        
        Serial.println(F("Reliability of Transmission"));
        
        Serial.print(F("    transmitCount: "));
        Serial.print(userConfig_.GetTransmitCount());
        Serial.println();
        
        Serial.print(F("    delaySecsBetweenTransmits: "));
        Serial.print(userConfig_.GetDelaySecsBetweenTransmits());
        Serial.println();
        Serial.println();
        
        
        Serial.println(F("Message Content"));
        Serial.print(F("    notesMessage["));
        Serial.print(UserConfig::NOTES_MESSAGE_LEN);
        Serial.print(F("]: "));
        Serial.print(userConfig_.GetNotesMessage());
        Serial.println();
        
        Serial.println(F("---------------------"));
    }

    UserConfig userConfig_;
};


#endif  // __APP_PICO_TRACKER_1_USER_CONFIG_SERIAL_INTERFACE_H__









