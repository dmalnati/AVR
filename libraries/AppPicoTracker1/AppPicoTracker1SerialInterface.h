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
                if (!strcmp_P(str.TokenAtIdx(1, ' '), PSTR("start")))
                {
                    Serial.println(F("GPS Start!!"));
                    tracker_.StartGPS();
                }
                else if (!strcmp_P(str.TokenAtIdx(1, ' '), PSTR("stop")))
                {
                    Serial.println(F("GPS Stop!!"));
                    tracker_.StopGPS();
                }
                else if (!strcmp_P(str.TokenAtIdx(1, ' '), PSTR("status")))
                {
                    SensorGPSUblox::Measurement measurement;
                    
                    uint8_t retValGps = tracker_.gps_.GetMeasurement(&measurement);
                    
                    Serial.print(F("GPS Status: "));
                    Serial.print(retValGps);
                    Serial.println();
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











