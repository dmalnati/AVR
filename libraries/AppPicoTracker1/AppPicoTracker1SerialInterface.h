#ifndef __APP_PICO_TRACKER_1_SERIAL_INTERFACE_H__
#define __APP_PICO_TRACKER_1_SERIAL_INTERFACE_H__


#include "UtlSerial.h"


class AppPicoTracker1SerialInterface
{
    static const uint8_t NUM_COMMANDS = 5;
    
    
public:
    
    AppPicoTracker1SerialInterface(AppPicoTracker1 &tracker)
    : tracker_(tracker)
    {
        sas_.RegisterCommand("gps", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                if (!strcmp_P(str.TokenAtIdx(1, ' '), PSTR("enable")))
                {
                    Serial.println(F("GPS Enable!!"));
                    tracker_.GPSEnable();
                }
                else if (!strcmp_P(str.TokenAtIdx(1, ' '), PSTR("disable")))
                {
                    Serial.println(F("GPS Disable!!"));
                    tracker_.GPSDisable();
                }
            }
        });
    }
    
    void Start()
    {
        sas_.Start();
    }
    
    void Stop()
    {
        sas_.Stop();
    }


private:
    
    AppPicoTracker1 &tracker_;

    SerialAsyncConsoleEnhanced<NUM_COMMANDS> sas_;
};

#include "AppPicoTracker1.h"

#include "AppPicoTracker1SerialInterface.h"



#endif  // __APP_PICO_TRACKER_1_SERIAL_INTERFACE_H__











