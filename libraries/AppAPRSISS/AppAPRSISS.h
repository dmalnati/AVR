#ifndef __APP_APRS_ISS_H__
#define __APP_APRS_ISS_H__


#include "PAL.h"
#include "Log.h"
#include "Evm.h"
#include "AppAPRSISSTestableBase.h"
#include "AppAPRSISSUserConfigManager.h"

//
// Runtime UI consists of:
// - Green LED
// - Red LED
// - Button
//
// The green LED indicates GPS lock state.
//   Blinking = not locked
//   Solid on = locked
//
// The red LED indicates sending.
//   On = sending
//   Off = not sending
//
// The button sends an APRS message w/ the user-configured parameters.
//


class AppAPRSISS
: public AppAPRSISSTestableBase
{
private:


public:

    AppAPRSISS(const AppAPRSISSConfig &cfg)
    : AppAPRSISSTestableBase(cfg)
    {
        // Nothing to do
    }
    
    void Run()
    {
        AppAPRSISSTestableBase::Init();
        
        // Blink to indicate power on
        Blink(cfg_.pinLedRed,   100);
        Blink(cfg_.pinLedGreen, 100);

        // Interact with user if present
        uint8_t userConfigOk = 0;
        
        // Keep the sizable manager out of memory indefinitely by only creating
        // a temporary instance.
        //
        // At this point during startup, there is no other competition for ram.
        {
            AppAPRSISSUserConfigManager mgr(cfg_.pinConfigure, userConfig_);
            
            // For use in testing out different configurations
            uint8_t letDefaultApplyAutomatically = 0;
            userConfigOk = mgr.GetConfig(letDefaultApplyAutomatically);
        }

        if (userConfigOk)
        {
            // Start the process of getting a GPS lock
            StartGPS();
            tedCheckGpsLock_.SetCallback([this](){
                OnCheckGpsLockState();
            });
            tedCheckGpsLock_.RegisterForTimedEventInterval(1000, 0);


            // Handle user button press to send
            pinInputSend_.SetCallback([this](uint8_t){
                OnPressSend();
            });
            pinInputSend_.Enable();

            
            // Handle async events
            Log(P("Running"));
            evm_.MainLoop();
        }
        else
        {
            Log(P("Bad Configuration"));
            
            while (1)
            {
                Blink(cfg_.pinLedRed, 500);
            }
        }
    }
    
private:

    void OnCheckGpsLockState()
    {
        SensorGPSUblox::Measurement m;

        if (gps_.GetLocationMeasurement(&m))
        {
            PAL.DigitalWrite(cfg_.pinLedGreen, HIGH);

            // If we lock once, that's good enough, it'll update later on
            // if new location acquired but don't need to start
            // blinking again when GPS is stopped and re-started.
            tedCheckGpsLock_.DeRegisterForTimedEvent();
        }
        else
        {
            PAL.DigitalToggle(cfg_.pinLedGreen);
        }
    }

    void OnPressSend()
    {
        Log(P("OnPressSend"));
        
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_8000_MS);
        PAL.DigitalWrite(cfg_.pinLedRed, HIGH);
        
        SensorGPSUblox::Measurement m;
        gps_.GetLocationMeasurement(&m);

        // Need to prevent the GPS from sending text while transmitting.
        // It causes interrupts to fire and mess up the timing of the
        // signal to the radio.
        StopGPS();

        SendMessage(m);

        StartGPS();

        PAL.DigitalWrite(cfg_.pinLedRed, LOW);
        PAL.WatchdogDisable();
    }

    
    void Blink(uint8_t pin, uint32_t durationMs)
    {
        PAL.DigitalWrite(pin, HIGH);
        PAL.Delay(durationMs);
        PAL.DigitalWrite(pin, LOW);
        PAL.Delay(durationMs);
    }
    

private:

    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
    TimedEventHandlerDelegate tedCheckGpsLock_;
};


#endif  // __APP_APRS_ISS_H__




























