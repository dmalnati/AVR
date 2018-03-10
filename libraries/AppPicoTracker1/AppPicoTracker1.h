#ifndef __APP_PICO_TRACKER_1_H__
#define __APP_PICO_TRACKER_1_H__


#include "Eeprom.h"
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
    , gps_(cfg_.pinGpsSerialRx, cfg_.pinGpsSerialTx)
    , radio_(cfg_.pinRadioSlaveSelect, cfg_.pinRadioShutdown)
    {
        // Nothing to do
    }
    
    void Run()
    {
        // Init serial
        Serial.begin(9600);
        if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_WATCHDOG)
        {
            Serial.println(F("WDTR"));
        }
        Serial.println(F("Starting"));
        
        
        // Maintain counters
        persistentCountersAccessor_.Read(persistentCounters_);
        
        ++persistentCounters_.NUM_RESTARTS;
        if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_WATCHDOG)
        {
            ++persistentCounters_.NUM_WDT_RESTARTS;
        }
        
        persistentCountersAccessor_.Write(persistentCounters_);
        
        Serial.print(persistentCounters_.NUM_RESTARTS);
        Serial.print(" ");
        Serial.print(persistentCounters_.NUM_WDT_RESTARTS);
        Serial.println();

        
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
            // Set up radio
            
            
            // Set up APRS Message Sender
            radio_.Init();
            radio_.SetFrequency(144390000);
            amt_.Init([this](){ radio_.Start(); }, [this](){ radio_.Stop(); });
            
            // Begin tracker reporting
            StartReportInterval();
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
        // Begin monitoring code which has been seen to hang
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_8000_MS);
        
        Serial.println(F("RIT"));
        
        // No need for low-power mode while we're attempting to do the primary
        // tracker behavior of lock and send.
        // Besides, we're going to async-spin waiting for a GPS lock, and in the
        // meantime we can use a watchdog
        //
        // TODO reconcile with serial-in logic
        //
        evm_.LowPowerDisable();
        
        // Start GPS
        StartGPS();
        
        // Keep track of when GPS started so you can track duration waiting
        transientCounters_.gpsFixWaitStart = PAL.Millis();
        
        // Turn off further interval timers until this async one completes
        tedReportIntervalTimeout_.DeRegisterForTimedEvent();

        // Start async waiting for GPS to lock
        Serial.println(F("Locking"));
        tedWaitForGpsLock_.SetCallback([this](){ OnCheckForGpsLock(); });
        tedWaitForGpsLock_.RegisterForTimedEventInterval(0);
        
        // Kick the watchdog
        PAL.WatchdogReset();
    }
        
    
    void OnCheckForGpsLock()
    {
        // Kick the watchdog
        PAL.WatchdogReset();
        
        // Check if GPS is locked on, saving the result in the process
        if (gps_.GetMeasurement(&gpsMeasurement_))
        {
            OnGpsLock();
        }
    }
    
    void OnGpsLock()
    {
        // Kick the watchdog
        PAL.WatchdogReset();

        // Cancel task to check for GPS lock
        tedWaitForGpsLock_.DeRegisterForTimedEvent();
        
        // Keep track of GPS counters
        transientCounters_.GPS_WAIT_FOR_FIX_DURATION_MS = (PAL.Millis() - transientCounters_.gpsFixWaitStart);
        
        
        // Measurement already cached during lock
        
        
        
        // Debug
        Serial.print(gpsMeasurement_.hour);
        Serial.print(":");
        Serial.print(gpsMeasurement_.minute);
        Serial.print(":");
        Serial.print(gpsMeasurement_.second);
        Serial.println();
        
        Serial.print(F("Wait: "));
        Serial.print(transientCounters_.GPS_WAIT_FOR_FIX_DURATION_MS);
        Serial.println();
        Serial.print(F("Alt: "));
        Serial.print(gpsMeasurement_.altitudeFt);
        Serial.println();
        
        // Turn off GPS
        StopGPS();
        
        // Send message
        SendMessage();
        
        // Re-start interval timer
        StartReportInterval();
        
        
        
        
        
        // Disable watchdog as the main set of code which can hang is complete
        PAL.WatchdogDisable();

        
        // Re-enable low-power mode, since the main async events are few and far
        // between at this point
        evm_.LowPowerEnable();
        
        
        // Debug
        Serial.println();
    }
    
    void SendMessage()
    {
        // Kick the watchdog
        PAL.WatchdogReset();
        
        AX25UIMessage &msg = *amt_.GetAX25UIMessage();

        msg.SetDstAddress("APZ001", 0);
        msg.SetSrcAddress("KD2KDD", 9);
        msg.AddRepeaterAddress("WIDE1", 1);

        // Add APRS data
        uint8_t *bufInfo    = NULL;
        uint8_t  bufInfoLen = 0;
        
        // Get buffer from AX25UIMessage to fill in APRS content
        if (msg.GetUnsafePtrInfo(&bufInfo, &bufInfoLen))
        {
            // Give buffer to helper interface
            APRSPositionReportMessagePicoTracker1 aprm;
            aprm.SetTargetBuf(bufInfo, bufInfoLen);
            
            
            // Fill out standard APRS fields
            aprm.SetTimeLocal(gpsMeasurement_.hour,
                              gpsMeasurement_.minute,
                              gpsMeasurement_.second);
            aprm.SetLatitude(gpsMeasurement_.latitudeDegrees,
                             gpsMeasurement_.latitudeMinutes,
                             gpsMeasurement_.latitudeSeconds);
            aprm.SetSymbolTableID('/');
            aprm.SetLongitude(gpsMeasurement_.longitudeDegrees,
                              gpsMeasurement_.longitudeMinutes,
                              gpsMeasurement_.longitudeSeconds);
            aprm.SetSymbolCode('O');
            
            
            // Fill out extended standard APRS fields
            aprm.SetCommentCourseAndSpeed(gpsMeasurement_.courseDegrees,
                                          gpsMeasurement_.speedKnots);
            aprm.SetCommentAltitude(gpsMeasurement_.altitudeFt);

            // Fill out my custom extensions
            ++transientCounters_.SEQ_NO;
            
            const char MESSAGE_TYPE_NORMAL = ' ';
            aprm.SetCommentMessageType(MESSAGE_TYPE_NORMAL);
            aprm.SetCommentSeqNoBinaryEncoded(transientCounters_.SEQ_NO);
            aprm.SetCommentGpsLockWaitSecsBinaryEncoded(transientCounters_.GPS_WAIT_FOR_FIX_DURATION_MS / 1000);
            aprm.SetCommentNumRestartsBinaryEncoded(persistentCounters_.NUM_RESTARTS);
            aprm.SetCommentNumWdtRestartsBinaryEncoded(persistentCounters_.NUM_WDT_RESTARTS);
            aprm.SetCommentNumMsgsNotSentBinaryEncoded(persistentCounters_.NUM_MSGS_NOT_SENT);
            
            // Update message structure to know how many bytes we used
            msg.AssertInfoBytesUsed(aprm.GetBytesUsed());
            
            
            // Debug
            Serial.print("U "); Serial.print(aprm.GetBytesUsed()); Serial.println();
        }


        // Configure and Transmit
        amt_.SetFlagStartDurationMs(300);
        amt_.SetFlagEndDurationMs(10);
        amt_.SetTransmitCount(1);
        amt_.SetDelayMsBetweenTransmits(2000);

        Serial.println("TX");
        
        // Kick the watchdog
        PAL.WatchdogReset();
        amt_.Transmit();
        
        Serial.println("-");
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    void StartGPS()
    {
        // enable power supply to GPS
        PAL.DigitalWrite(cfg_.pinGpsEnable, HIGH);
        
        // re-init to begin cycle again
        gps_.Init();
        
        // assert this is a high-altitude mode
        gps_.SetHighAltitudeMode();
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
        
        // saw at least one fatal hang
            // watchdog how?
            
        // how to sleep longer when way out of range?
    }
    
    
    
private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppPicoTracker1Config &cfg_;
    

    uint32_t                  reportIntervalMs_ = 1000;
    TimedEventHandlerDelegate tedReportIntervalTimeout_;
    
    
    TimedEventHandlerDelegate tedWaitForGpsLock_;
    
    
    SensorGPSUblox               gps_;
    SensorGPSUblox::Measurement  gpsMeasurement_;
    
    
    RFSI4463PRO                 radio_;
    AX25UIMessageTransmitter<>  amt_;
    
    
    
    struct PersistentCounters
    {
        uint16_t NUM_RESTARTS      = 0;
        uint16_t NUM_WDT_RESTARTS  = 0;
        uint16_t NUM_MSGS_NOT_SENT = 0;
    };
    
    PersistentCounters                 persistentCounters_;
    EepromAccessor<PersistentCounters> persistentCountersAccessor_;
    
    
    struct TransientCounters
    {
        uint32_t gpsFixWaitStart              = 0;
        uint32_t GPS_WAIT_FOR_FIX_DURATION_MS = 0;
        
        uint32_t SEQ_NO = 0;
    };
    
    TransientCounters  transientCounters_;
    
};









#endif  // __APP_PICO_TRACKER_1_H__




























