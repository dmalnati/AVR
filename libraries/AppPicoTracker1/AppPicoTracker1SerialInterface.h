#ifndef __APP_PICO_TRACKER_1_SERIAL_INTERFACE_H__
#define __APP_PICO_TRACKER_1_SERIAL_INTERFACE_H__


#include "UtlSerial.h"


// 2,948 bytes progmem
class AppPicoTracker1SerialInterface
{
    static const uint8_t NUM_COMMANDS = 2;
    
public:
    
    AppPicoTracker1SerialInterface(AppPicoTracker1 &tracker)
    : tracker_(tracker)
    {
        sas_.RegisterCommand("gps", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') >= 2)
            {
                if (!strcmp_P(str.TokenAtIdx(1, ' '), PSTR("start")))
                {
                    Serial.println(F("GPS Start!!"));
                    tracker_.StartGPS();
                }
                else if (!strcmp_P(str.TokenAtIdx(1, ' '), PSTR("stop")))
                {
                    uint8_t saveConfiguration = 1;
                    
                    if (str.TokenCount(' ') == 3)
                    {
                        saveConfiguration = atoi(str.TokenAtIdx(2, ' '));
                    }
                    
                    Serial.print(F("GPS Stop - "));
                    Serial.print(saveConfiguration);
                    Serial.println();
                    
                    tracker_.StopGPS(saveConfiguration);
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
        
        sas_.RegisterCommand("ri", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                uint32_t reportIntervalMs = atol(str.TokenAtIdx(1, ' '));
                
                Serial.print(F("ReportInterval "));
                Serial.print(reportIntervalMs);
                Serial.println();
                
                tracker_.SetReportInterval(reportIntervalMs);
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


#endif  // __APP_PICO_TRACKER_1_SERIAL_INTERFACE_H__











