#ifndef __APP_PICO_TRACKER_1_USER_CONFIG_SERIAL_INTERFACE_H__
#define __APP_PICO_TRACKER_1_USER_CONFIG_SERIAL_INTERFACE_H__


    // struct UserConfig
    // {
        // char    callsign[7] = { '\0', '\0', '\0', '\0', '\0', '\0', '\0',  };
        // uint8_t ssid        = 9;
        
        // uint32_t reportIntervalSecs = 60;
        
        // uint8_t  transmitCount             = 2;
        // uint32_t delaySecsBetweenTransmits = 3;
        
        // char notesMessage[25];
    // };



#include <avr/pgmspace.h>

#include "Eeprom.h"
#include "UtlSerial.h"


class AppPicoTracker1UserConfigSerialInterface
{
public:

    void GetUserConfig()
    {
        PrintHelp();
        
        const uint8_t BUF_SIZE = 100;
        char buf[BUF_SIZE];
        
        EepromAccessor<UserConfig> ea;
        ea.Read(userConfig_);
        
        while (1)
        {
            uint8_t strLen = SerialReadLine(buf, BUF_SIZE);
            
            if (strLen)
            {
                char *strP = NULL;
                
                if (!strcmp_P(buf, PSTR("help")) || !strcmp_P(buf, PSTR("?")))
                {
                    PrintHelp();
                }
                else if ((strP = strstr_P(buf, PSTR("callsign"))))
                {
                    
                    Serial.println("callsign!!");
                    
                    // need some string libs to help with trim left/right
                    // and splitting(?)
                    
                    
                }
                else
                {
                    Serial.print(F("Not understood: "));
                    Serial.print(buf);
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
        
        Serial.println(F("Your identity"));
        
        Serial.print(F("    callsign: "));
        Serial.print(userConfig_.callsign);
        Serial.println();
        
        Serial.print(F("    ssid: "));
        Serial.print(userConfig_.ssid);
        Serial.println();
        
        
        Serial.println(F("Reporting Interval"));
        
        Serial.print(F("    reportIntervalSecs: "));
        Serial.print(userConfig_.reportIntervalSecs);
        Serial.println();
        
        
        Serial.println(F("Reliability of Transmission"));
        
        Serial.print(F("    transmitCount: "));
        Serial.print(userConfig_.transmitCount);
        Serial.println();
        
        Serial.print(F("    delaySecsBetweenTransmits: "));
        Serial.print(userConfig_.delaySecsBetweenTransmits);
        Serial.println();
        
        
        Serial.println(F("Message Content"));
        
        Serial.print(F("    notesMessage: "));
        Serial.print(userConfig_.notesMessage);
        Serial.println();
        
    }

    UserConfig userConfig_;
};

#endif  // __APP_PICO_TRACKER_1_USER_CONFIG_SERIAL_INTERFACE_H__