#ifndef __APP_PICO_TRACKER_1_H__
#define __APP_PICO_TRACKER_1_H__


#include "Evm.h"
#include "LedBlinker.h"
//#include "AppPicoTracker1UserConfigManager.h"
#include "SensorGPSUblox.h"
#include "RFSI4463PRO.h"
#include "AX25UIMessageTransmitter.h"
#include "APRSPositionReportMessagePicoTracker1.h"


#include "UtlStreamBlob.h"

struct AppPicoTracker1Config
{
    // GPS
    uint8_t pinGpsEnable;
    uint8_t pinGpsSerialRx; // receive GPS data from this pin
    uint8_t pinGpsSerialTx; // send data to the GPS on this pin
    
    // Status LEDs
    uint8_t pinLedRunning;
    uint8_t pinLedGpsLocked;
    uint8_t pinLedTransmitting;
    
    // Radio
    uint8_t pinRadioSlaveSelect;
    uint8_t pinRadioShutdown;
};

class AppPicoTracker1
{
private:
    static const uint32_t DURATION_MS_CHECK_SERIAL_INPUT_ACTIVE = 5000;
    
    static const uint32_t DURATION_MS_LED_RUNNING_OFF_NORMAL = 4950;
    static const uint32_t DURATION_MS_LED_RUNNING_ON_NORMAL  =   50;
    
    static const uint32_t DURATION_MS_LED_RUNNING_OFF_SERIAL = 950;
    static const uint32_t DURATION_MS_LED_RUNNING_ON_SERIAL  =  50;
    
    static const uint32_t DURATION_MS_LED_GPS_OFF = 950;
    static const uint32_t DURATION_MS_LED_GPS_ON  =  50;
    
    static const uint8_t PIN_SERIAL_RX = 2;
        
private:
    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
public:
    AppPicoTracker1(AppPicoTracker1Config &cfg)
    : cfg_(cfg)
    , serIface_(*this)
    , serialInputActive_(0)
    , ledBlinkerRunning_(cfg_.pinLedRunning)
    , ledBlinkerGps_(cfg_.pinLedGpsLocked)
    , gps_(cfg_.pinGpsSerialRx, cfg_.pinGpsSerialTx)
    , radio_(cfg_.pinRadioSlaveSelect, cfg_.pinRadioShutdown)
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
        
        //if (AppPicoTracker1UserConfigManager::GetUserConfig(userConfig_) || 1)
        if (1)
        {
            Serial.println();
            Serial.println(F("Proceeding with:"));
            //userConfig_.Print();
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
            // Set up blinking
            ledBlinkerRunning_.SetDurationOffOn(DURATION_MS_LED_RUNNING_OFF_NORMAL,
                                                DURATION_MS_LED_RUNNING_ON_NORMAL);
            ledBlinkerRunning_.Start();
            
            // Watch for serial input, check immediately
            tedSerialMonitor_.SetCallback([this](){
                OnCheckForSerialInputActive();
            });
            tedSerialMonitor_.RegisterForTimedEventInterval(DURATION_MS_CHECK_SERIAL_INPUT_ACTIVE);
            tedSerialMonitor_();
            
            // Set up radio
            
            
            // Set up APRS Message Sender
            radio_.Init();
            radio_.SetFrequency(144390000);
            amt_.Init([this](){ radio_.Start(); }, [this](){ radio_.Stop(); });
            
            // Begin tracker reporting
            StartReportInterval();
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
    
    void StartReportInterval()
    {
        tedReportIntervalTimeout_.SetCallback([this](){
            OnReportIntervalTimeout();
        });

        tedReportIntervalTimeout_.RegisterForTimedEventInterval(reportIntervalMs_);
    }
    
    void SetReportInterval(uint32_t reportIntervalMs)
    {
        reportIntervalMs_ = reportIntervalMs;
    }

    
    void OnReportIntervalTimeout()
    {
        Serial.println(F("ReportIntervalTimeout"));
        
        // Start GPS
        StartGPS();
        
        // Keep track of when GPS started so you can track duration waiting
        counters_.gpsFixWaitStart = PAL.Millis();
        
        // Start async waiting for GPS to lock
        Serial.println(F("Attempting GPS Lock"));
        tedWaitForGpsLock_.SetCallback([this](){ OnCheckForGpsLock(); });
        tedWaitForGpsLock_.RegisterForTimedEventInterval(0);
        
        // Turn off further interval timers until this async one completes
        tedReportIntervalTimeout_.DeRegisterForTimedEvent();
    }
    
    void OnCheckForGpsLock()
    {
        if (gps_.GetMeasurement(&gpsMeasurement_))
        {
            OnGpsLock();
        }
    }
    
    void OnGpsLock()
    {
        // Keep track of GPS stats
        counters_.GPS_WAIT_FOR_FIX_DURATION_MS = (PAL.Millis() - counters_.gpsFixWaitStart);
        counters_.GPS_WAIT_FOR_FIX_DURATION_MS_TOTAL += counters_.GPS_WAIT_FOR_FIX_DURATION_MS;
        
        // Get measurement
        CacheGpsMeasurement();
        Serial.print(F("Wait: "));
        Serial.print(counters_.GPS_WAIT_FOR_FIX_DURATION_MS);
        Serial.println();
        Serial.print(F("TWait: "));
        Serial.print(counters_.GPS_WAIT_FOR_FIX_DURATION_MS_TOTAL);
        Serial.println();
        Serial.print(F("Alt: "));
        Serial.print(gpsMeasurement_.altitudeFt);
        Serial.println();
        
        // Turn off GPS
        Serial.println(F("Disabling GPS"));
        StopGPS();
        
        // Cancel task to check for GPS lock
        tedWaitForGpsLock_.DeRegisterForTimedEvent();
        
        // Send message
        SendMessage();
        
        // Re-start interval timer
        StartReportInterval();
    }
    
    void SendMessage()
    {
        AX25UIMessage &msg = *amt_.GetAX25UIMessage();

        msg.SetDstAddress("APZ001", 0);
        msg.SetSrcAddress("KD2KDD", 9);
        msg.AddRepeaterAddress("WIDE1", 1);

        // Add APRS data
        uint8_t *bufInfo    = NULL;
        uint8_t  bufInfoLen = 0;
        
        APRSPositionReportMessagePicoTracker1 aprm;

        if (msg.GetUnsafePtrInfo(&bufInfo, &bufInfoLen))
        {
            aprm.SetTargetBuf(bufInfo, bufInfoLen);
        
            aprm.SetTimeLocal(19, 14, 7);
            aprm.SetLatitude(40, 44, 13.87);
            aprm.SetSymbolTableID('/');
            aprm.SetLongitude(-74, 2, 2.32);
            aprm.SetSymbolCode('O');
            
            // extended
            aprm.SetCommentCourseAndSpeed(273, 777);
            aprm.SetCommentAltitude(444);

            // my extensions
            aprm.SetCommentBarometricPressureBinaryEncoded(10132);   // sea level
            aprm.SetCommentTemperatureBinaryEncoded(72); // first thermometer, inside(?)
            aprm.SetCommentMagneticsBinaryEncoded(-0.2051, 0.0527, 0.0742);    // on my desk
            aprm.SetCommentAccelerationBinaryEncoded(56.7017, 1042.7856, -946.2891);    // on my desk, modified y
            aprm.SetCommentTemperatureBinaryEncoded(74); // the other thermometer, outside(?)
            aprm.SetCommentVoltageBinaryEncoded(4.723);

            static uint16_t seqNo = 0;
            aprm.SetCommentSeqNoBinaryEncoded(++seqNo);

            msg.AssertInfoBytesUsed(aprm.GetBytesUsed());
        }


        // Configure and Transmit
        amt_.SetFlagStartDurationMs(300);
        amt_.SetFlagEndDurationMs(10);
        amt_.SetTransmitCount(1);
        amt_.SetDelayMsBetweenTransmits(2000);

        static uint32_t timeLast = 0;
        Serial.print("Transmitting - ");
        uint32_t timeNow = PAL.Millis();
        uint32_t timeDiff = timeNow - timeLast;
        timeLast = timeNow;
        Serial.print(timeDiff);
        Serial.println(" ms since last");
        
        amt_.Transmit();
    }
    
    
    
    
    
    
    
    void StartGPS()
    {
        // enable power supply to GPS
        PAL.DigitalWrite(cfg_.pinGpsEnable, HIGH);
        
        // re-init to begin cycle again
        gps_.Init();
        
        // assert this is a high-altitude mode
        gps_.SetHighAltitudeMode();
        
        // get blinker indicating a lock is being attempted
        ledBlinkerGps_.SetDurationOffOn(DURATION_MS_LED_GPS_OFF,
                                        DURATION_MS_LED_GPS_ON);
        ledBlinkerGps_.Start();
    }
    
    
    void StopGPS(uint8_t saveConfiguration = 1)
    {
        // Don't allow prior fix to be used, we want a brand new fix to be
        // acquired next time the GPS starts up
        gps_.ResetFix();
        
        // stop interrups from firing in underlying code
        gps_.DisableSerialInput();
        
        if (saveConfiguration)
        {
            // cause the gps module to store the metadata is has learned from
            // the satellites it can see and used to get a lock.
            // this will be read again automatically by the module on startup.
            gps_.SaveConfiguration();
        }
        
        // disable power supply to GPS
        // (battery backup for module-stored data supplied through other pin)
        PAL.DigitalWrite(cfg_.pinGpsEnable, LOW);
        
        // clear GPS lock status indicator
        ledBlinkerGps_.Stop();
    }
    
    
    
    
    
    
    
    
    
    
    void CacheGpsMeasurement()
    {
        gps_.GetMeasurement(&gpsMeasurement_);
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
            // internal voltage wasn't working correctly
            
        // GPS geofencing to change frequency
        
        // Make sure to manually enable high-altitude mode
        
        // use LEDs as status
            // different schemes for config vs low-altitude status
            // disable when flying
        
        // set serial active checking frequency more appropriately
            // confirm working and won't eat power by waking up inappropriately
            // during actual flight
        
        // watchdog
        
        // RTC
        
        // dump flash stored locations if found later
        
        
        // restore fuse setting to erase EEPROM on re-program
            // temporarily disabled for development
            
        // restore user config being sensitive to RX-in
        // restore user config check (has || 1)
        
        // user-configure whether LEDs blink or not for status
        
        
        // things to include in message:
        // - voltage
        // - time to get latest fix
        // stats
            // - number unsent messages since last tx
                // - maybe better to keep absolute counters
            // - number of restarts
            // - uptime
        
    }
    
    
    
private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppPicoTracker1Config &cfg_;
    
    #include "AppPicoTracker1SerialInterface.h"
    AppPicoTracker1SerialInterface serIface_;
    
    //AppPicoTracker1UserConfigManager::UserConfig userConfig_;
    
    
    uint32_t                  reportIntervalMs_ = 5000;
    TimedEventHandlerDelegate tedReportIntervalTimeout_;
    
    
    TimedEventHandlerDelegate tedSerialMonitor_;
    uint8_t                   serialInputActive_;
    
    LedBlinker  ledBlinkerRunning_;
    LedBlinker  ledBlinkerGps_;
    
    
    TimedEventHandlerDelegate tedWaitForGpsLock_;
    
    
    SensorGPSUblox               gps_;
    SensorGPSUblox::Measurement  gpsMeasurement_;
    
    
    RFSI4463PRO                 radio_;
    AX25UIMessageTransmitter<>  amt_;
    
    
    struct CountersState
    {
        uint32_t gpsFixWaitStart = 0;
        
        uint32_t GPS_WAIT_FOR_FIX_DURATION_MS = 0;
        uint32_t GPS_WAIT_FOR_FIX_DURATION_MS_TOTAL = 0;
    };

    CountersState counters_;
    
    
};









#endif  // __APP_PICO_TRACKER_1_H__




























