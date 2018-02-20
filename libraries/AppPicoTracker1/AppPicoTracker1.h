#ifndef __APP_PICO_TRACKER_1_H__
#define __APP_PICO_TRACKER_1_H__


#include "Evm.h"
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
    static const uint32_t DURATION_MS_LED_RUNNING_BLINK_NORMAL  = 1000;
    static const uint32_t DURATION_MS_LED_RUNNING_BLINK_SERIAL  = 250;
    
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
    {
        // Nothing to do
    }
    
    void Run()
    {
        Serial.begin(9600);
        Serial.println(F("Starting"));
        
        // Set up status LEDs
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
            tedLedRunning_.SetCallback([this](){
                OnLedRunningTimeout();
            });
            tedLedRunning_.RegisterForTimedEventInterval(DURATION_MS_LED_RUNNING_BLINK_NORMAL);
            
            // Watch for serial input, check immediately
            tedSerialMonitor_.SetCallback([this](){
                OnCheckForSerialInputActive();
            });
            tedSerialMonitor_.RegisterForTimedEventInterval(DURATION_MS_CHECK_SERIAL_INPUT_ACTIVE);
            tedSerialMonitor_();
        }
        else if (evt == Event::SERIAL_INPUT_ACTIVE)
        {
            // blink immediately and run at different interval
            tedLedRunning_();
            tedLedRunning_.RegisterForTimedEventInterval(DURATION_MS_LED_RUNNING_BLINK_SERIAL);
        }
        else if (evt == Event::SERIAL_INPUT_INACTIVE)
        {
            // blink immediately and run at different interval
            tedLedRunning_();
            tedLedRunning_.RegisterForTimedEventInterval(DURATION_MS_LED_RUNNING_BLINK_NORMAL);
        }
    }


    void OnLedRunningTimeout()
    {
        PAL.DigitalToggle(cfg_.pinLedRunning);
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
    }
    
    
    
private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppPicoTracker1Config &cfg_;
    
    AppPicoTracker1UserConfigManager::UserConfig userConfig_;
    
    TimedEventHandlerDelegate tedSerialMonitor_;
    uint8_t                   serialInputActive_;
    
    TimedEventHandlerDelegate tedLedRunning_;
};


#endif  // __APP_PICO_TRACKER_1_H__




























