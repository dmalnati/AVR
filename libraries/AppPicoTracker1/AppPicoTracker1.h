#ifndef __APP_PICO_TRACKER_1_H__
#define __APP_PICO_TRACKER_1_H__


#include "Evm.h"



struct AppPicoTracker1Config
{
};

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
    
    uint8_t GetSsid() { return ssid; }
    void SetSsid(uint8_t val) { ssid = val < 16 ? val : 15; }
    
    uint32_t GetReportIntervalSecs() { return reportIntervalSecs; }
    void SetReportIntervalSecs(uint32_t val) { reportIntervalSecs = val; }
    
    uint8_t GetTransmitCount() { return transmitCount; }
    void SetTransmitCount(uint8_t val) { transmitCount = val; }
    
    uint32_t GetDelaySecsBetweenTransmits() { return delaySecsBetweenTransmits; }
    void SetDelaySecsBetweenTransmits(uint32_t val) { delaySecsBetweenTransmits = val; }

    char *GetNotesMessage() { return notesMessage; }
    void SetNotesMessage(const char *val) { strncpy(notesMessage, val, NOTES_MESSAGE_LEN + 1); notesMessage[NOTES_MESSAGE_LEN] = '\0'; }
    
private:
    char deviceId[DEVICE_ID_LEN + 1];
    
    char    callsign[CALLSIGN_LEN + 1];
    uint8_t ssid = 9;
    
    uint32_t reportIntervalSecs = 60;
    
    uint8_t  transmitCount             = 2;
    uint32_t delaySecsBetweenTransmits = 3;
    
    char notesMessage[NOTES_MESSAGE_LEN + 1];
};

#include "AppPicoTracker1UserConfigSerialInterface.h"

class AppPicoTracker1
{
private:
    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;
    
public:

    
public:
    AppPicoTracker1(AppPicoTracker1Config &cfg)
    : cfg_(cfg)
    {
        // Nothing to do
    }
    
    void Run()
    {
        Serial.begin(9600);
        Serial.println(F("Starting"));
        
        PAL.Delay(20);
        
        if (PAL.DigitalRead(2))
        {
            Serial.println(F("Config mode enabled"));
            
            AppPicoTracker1UserConfigSerialInterface menu;
            
            menu.GetUserConfig();
        }
        else
        {
            Serial.println(F("Config mode not in use"));
        }
        
        while (1) {}
        
        // blink error if no valid configuration found
        
        
        
        // wtf were the bugs I had before?
            // Something about the deg min sec not accounting for 00 and
            // turning into just spaces right?
            
        // GPS geofencing to change frequency
    }

private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppPicoTracker1Config &cfg_;
};


#endif  // __APP_PICO_TRACKER_1_H__