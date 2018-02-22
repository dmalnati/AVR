#ifndef __APP_PICO_TRACKER_1_H__
#define __APP_PICO_TRACKER_1_H__


#include "Evm.h"
#include "LedBlinker.h"
#include "AppPicoTracker1UserConfigManager.h"




struct AppPicoTracker1Config
{
    uint8_t pinGpsEnable;
    
    uint8_t pinLedRunning;
    uint8_t pinLedGpsLocked;
    uint8_t pinLedTransmitting;
};

class AppPicoTracker1
{
private:
    static const uint32_t DURATION_MS_CHECK_SERIAL_INPUT_ACTIVE = 5000;
    
    static const uint32_t DURATION_MS_LED_RUNNING_OFF_NORMAL = 4950;
    static const uint32_t DURATION_MS_LED_RUNNING_ON_NORMAL  =   50;
    
    static const uint32_t DURATION_MS_LED_RUNNING_OFF_SERIAL = 950;
    static const uint32_t DURATION_MS_LED_RUNNING_ON_SERIAL  =  50;
    
    static const uint8_t PIN_SERIAL_RX = 2;
    
private:
    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;
    
public:

    
public:
    AppPicoTracker1(AppPicoTracker1Config &cfg)
    : cfg_(cfg)
    , serialInputActive_(0)
    , ledBlinkerRunning_(cfg_.pinLedRunning)
    , serIface_(*this)
    {
        // Nothing to do
    }
    
    void Run()
    {
        Serial.begin(9600);
        Serial.println(F("Starting"));
        
        // Set up GPS enable pin
        PAL.PinMode(cfg_.pinGpsEnable, OUTPUT);
        
        // Set up status LED pins
        PAL.PinMode(cfg_.pinLedRunning,      OUTPUT);
        PAL.PinMode(cfg_.pinLedGpsLocked,    OUTPUT);
        PAL.PinMode(cfg_.pinLedTransmitting, OUTPUT);
        
        if (AppPicoTracker1UserConfigManager::GetUserConfig(userConfig_))
        {
            Serial.println();
            Serial.println(F("Proceeding with:"));
            userConfig_.Print();
            Serial.println();
            
            
            // Drive state machine
            OnEvent(Event::START);


            
            Serial.println(F("Running."));
            Serial.println();
            PAL.Delay(1000);
            
            // Handle async events
            evm_.MainLoopLowPower();
        }
        else
        {
            Serial.println();
            Serial.println(F("ERR: Invalid configuration, please restart and try again."));
            Serial.println();
            
            // blink error -- cannot continue without config
            while (1)
            {
                const uint32_t delayMs = 500;
                
                PAL.DigitalWrite(cfg_.pinLedRunning, HIGH);
                PAL.Delay(delayMs);
                PAL.DigitalWrite(cfg_.pinLedRunning, LOW);
                PAL.DigitalWrite(cfg_.pinLedGpsLocked, HIGH);
                PAL.Delay(delayMs);
                PAL.DigitalWrite(cfg_.pinLedGpsLocked, LOW);
                PAL.DigitalWrite(cfg_.pinLedTransmitting, HIGH);
                PAL.Delay(delayMs);
                PAL.DigitalWrite(cfg_.pinLedTransmitting, LOW);
            }
        }
    }

private:

    enum class Event : uint8_t
    {
        START,
        SERIAL_INPUT_ACTIVE,
        SERIAL_INPUT_INACTIVE,
    };

    void OnEvent(Event evt)
    {
        if (evt == Event::START)
        {
            // set up blinking
            ledBlinkerRunning_.SetDurationOffOn(DURATION_MS_LED_RUNNING_OFF_NORMAL,
                                                DURATION_MS_LED_RUNNING_ON_NORMAL);
            ledBlinkerRunning_.Start();
            
            // Watch for serial input, check immediately
            tedSerialMonitor_.SetCallback([this](){
                OnCheckForSerialInputActive();
            });
            tedSerialMonitor_.RegisterForTimedEventInterval(DURATION_MS_CHECK_SERIAL_INPUT_ACTIVE);
            tedSerialMonitor_();
        }
        else if (evt == Event::SERIAL_INPUT_ACTIVE)
        {
            Serial.println(F("Serial active"));
            
            // as long as serial input active, we need to be able to read from
            // it, so disable power saving
            evm_.LowPowerDisable();
            
            // run at different interval
            ledBlinkerRunning_.SetDurationOffOn(DURATION_MS_LED_RUNNING_OFF_SERIAL,
                                                DURATION_MS_LED_RUNNING_ON_SERIAL);
                                                
            // since uart active, can monitor for input
            serIface_.Start();
        }
        else if (evt == Event::SERIAL_INPUT_INACTIVE)
        {
            Serial.println(F("Serial inactive"));
            PAL.Delay(50);
            
            // serial input no longer active, enable power saving
            evm_.LowPowerEnable();
            
            // run at different interval
            ledBlinkerRunning_.SetDurationOffOn(DURATION_MS_LED_RUNNING_OFF_NORMAL,
                                                DURATION_MS_LED_RUNNING_ON_NORMAL);
            
            
            // no point in this polling anymore, no serial data will be received
            // since the uart isn't active in low power mode
            serIface_.Stop();
        }
    }
    
    void GPSEnable()
    {
        PAL.DigitalWrite(cfg_.pinGpsEnable, HIGH);
    }
    
    void GPSDisable()
    {
        PAL.DigitalWrite(cfg_.pinGpsEnable, LOW);
    }
    
    
    


    void OnCheckForSerialInputActive()
    {
        // found 2 empirically, added 1 for margin
        const uint32_t delayMsToWaitForPinToSettle = 2 + 1;
        
        PAL.Delay(delayMsToWaitForPinToSettle);
        
        uint8_t serialInputActive = PAL.DigitalRead(PIN_SERIAL_RX);
        
        if (serialInputActive == 1 && serialInputActive_ == 0)
        {
            // active now but wasn't before
            OnEvent(Event::SERIAL_INPUT_ACTIVE);
        }
        else if (serialInputActive == 0 && serialInputActive_ == 1)
        {
            // not active but was before
            OnEvent(Event::SERIAL_INPUT_INACTIVE);
        }
        
        serialInputActive_ = serialInputActive;
    }


    
    void ToDo()
    {
        
        // Change SSID to 11, not 9.
            // I think I set the icon display based on other characters in the
            // aprs message.
                // easy to expose to settings?  dad might like that.
        
        // APRS route should be WIDE2-1 not WIDE1-1, right?
        
        
        // wtf were the bugs I had before?
            // Something about the deg min sec not accounting for 00 and
            // turning into just spaces right?
            
        // GPS geofencing to change frequency
        
        // Make sure to manually enable high-altitude mode
        
        // use LEDs as status
            // different schemes for config vs low-altitude status
            // disable when flying
        
        // set serial active checking frequency more appropriately
        
        // watchdog
        
        // RTC
        
        // dump flash stored locations if found later
    }
    
    
    
private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppPicoTracker1Config &cfg_;
    
    AppPicoTracker1UserConfigManager::UserConfig userConfig_;
    
    TimedEventHandlerDelegate tedSerialMonitor_;
    uint8_t                   serialInputActive_;
    
    LedBlinker  ledBlinkerRunning_;
    
    #include "AppPicoTracker1SerialInterface.h"
    AppPicoTracker1SerialInterface serIface_;
};









#endif  // __APP_PICO_TRACKER_1_H__




























