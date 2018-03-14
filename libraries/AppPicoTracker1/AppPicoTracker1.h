#ifndef __APP_PICO_TRACKER_1_H__
#define __APP_PICO_TRACKER_1_H__


#include "Eeprom.h"
#include "Evm.h"
#include "LedBlinker.h"
#include "SensorGPSUblox.h"
#include "RFSI4463PRO.h"
#include "AX25UIMessageTransmitter.h"
#include "GeofenceAPRS.h"
#include "AppPicoTracker1UserConfigManager.h"
#include "APRSPositionReportMessagePicoTracker1.h"



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
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Entry point
    //
    ///////////////////////////////////////////////////////////////////////////

    void Run()
    {
        // Init serial and announce startup
        Serial.begin(9600);
        Serial.println(F("Starting"));
        if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_WATCHDOG)
        {
            Serial.println(F("WDTR"));
        }
        
        // Set up LED pins as output
        PAL.PinMode(cfg_.pinLedRunning,      OUTPUT);
        PAL.PinMode(cfg_.pinLedGpsLocked,    OUTPUT);
        PAL.PinMode(cfg_.pinLedTransmitting, OUTPUT);
        
        // Blink to indicate power on
        Blink(cfg_.pinLedRunning,      100);
        Blink(cfg_.pinLedGpsLocked,    100);
        Blink(cfg_.pinLedTransmitting, 100);

        // Get user config
        if (AppPicoTracker1UserConfigManager::GetUserConfig(userConfig_))
        {
            // Blink to indicate good configuration
            for (uint8_t i = 0; i < 3; ++i)
            {
                Blink(cfg_.pinLedGpsLocked, 300);
            }
            
            RunInternal();
        }
        else
        {
            // Blink to indicate bad configuration
            while (1)
            {
                Blink(cfg_.pinLedRunning, 300);
            }
        }
    }
    
private:

    void Blink(uint8_t pin, uint32_t durationMs)
    {
        PAL.DigitalWrite(pin, HIGH);
        PAL.Delay(durationMs);
        PAL.DigitalWrite(pin, LOW);
        PAL.Delay(durationMs);
    }

    void RunInternal()
    {
        // Maintain counters about restarts
        StatsIncrNumRestarts();
        if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_WATCHDOG)
        {
            StatsIncrNumWdtRestarts();
        }
        
        // Set up GPS enable pin
        PAL.PinMode(cfg_.pinGpsEnable, OUTPUT);
        
        // Set up radio
        radio_.Init();
        
        // Set up APRS Message Sender
        amt_.Init([this](){ radio_.Start(); }, [this](){ radio_.Stop(); });
        
        // Begin tracker reporting, fire first event immediately.
        // No point using interval, the decision about how long to sleep for is
        // evaluated each time.
        tedWakeAndEvaluateTimeout_.SetCallback([this](){
            OnWakeAndEvaluateTimeout();
        });
        tedWakeAndEvaluateTimeout_.RegisterForTimedEvent(0);
        
        // Handle async events
        Serial.println(F("Running"));
        PAL.Delay(1000);
        
        evm_.MainLoopLowPower();
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Main Wake/Lock/Send logic
    //
    ///////////////////////////////////////////////////////////////////////////

    void OnWakeAndEvaluateTimeout()
    {
        // Log
        Serial.println(" \nWake");

        // Begin monitoring code which has been seen to hang
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_8000_MS);
        
        // No need for low-power mode while we're attempting to do the primary
        // tracker behavior of lock and send.
        // Besides, we're going to async-spin waiting for a GPS lock, and in the
        // meantime we can use a watchdog
        evm_.LowPowerDisable();
        
        // Start GPS
        Serial.println("GPS ON");
        StartGPS();
        
        // Keep track of when GPS started so you can track duration waiting
        transientCounters_.gpsFixWaitStart = PAL.Millis();
        
        // Start async waiting for GPS to lock
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
            // Cancel task to check for GPS lock
            tedWaitForGpsLock_.DeRegisterForTimedEvent();
            
            // Notify that lock acquired
            OnGpsLock();
        }
    }
    
    void OnGpsLock()
    {
        // Kick the watchdog
        PAL.WatchdogReset();

        // Keep track of GPS counters
        transientCounters_.GPS_WAIT_FOR_FIX_DURATION_MS = (PAL.Millis() - transientCounters_.gpsFixWaitStart);
        
        // Turn off GPS
        Serial.println("GPS OFF");
        StopGPS();
        
        // Consult with Geofence to determine details about where we are
        GeofenceAPRS::LocationDetails locationDetails = 
            geofence_.GetLocationDetails(gpsMeasurement_.latitudeDegreesMillionths,
                                         gpsMeasurement_.longitudeDegreesMillionths);
        
        // Decide whether to send a message and how long to sleep for based on
        // geofence data.
        uint8_t sendMessage = 1;
        
        if (gpsMeasurement_.altitudeFt < userConfig_.geo.lowHighAltitudeFtThreshold)
        {
            // Debug
            // Serial.println(F("LowAltitude"));
            
            // At low altitude, we send messages regardless of being in a dead zone or not
            sendMessage = 1;
            
            // Wake again at the interval configured for low altitude
            tedWakeAndEvaluateTimeout_.RegisterForTimedEvent(userConfig_.geo.lowAltitude.wakeAndEvaluateMs);
        }
        else
        {
            // Debug
            // Serial.println(F("HighAltitude"));
            
            if (locationDetails.deadZone)
            {
                // Debug
                // Serial.println(F("DeadZone"));
                
                // At high altitude, we don't send messages in dead zones
                sendMessage = 0;
                
                // Increment counters to indicate this has happened.
                StatsIncrNumMsgsNotSent();
                
                // Wake again at the interval configured for high altitude dead zones
                tedWakeAndEvaluateTimeout_.RegisterForTimedEvent(userConfig_.geo.deadZone.wakeAndEvaluateMs);
            }
            else
            {
                // Debug
                // Serial.println(F("ActiveZone"));
                
                // At high altitude, we do send messages in active zones
                sendMessage = 1;
                
                // Wake again at the interval configured for high altitude active zones
                tedWakeAndEvaluateTimeout_.RegisterForTimedEvent(userConfig_.geo.highAltitude.wakeAndEvaluateMs);
            }
        }
        
        if (sendMessage)
        {
            // We're going to send a message, so tune radio to the frequency to
            // be used in this region.
            radio_.SetFrequency(locationDetails.freqAprs);
            
            // Log
            Serial.println("TX");
            
            // Send message
            SendMessage();
            
            // Log
            Serial.println("TXEND");
        }
        
        // Disable watchdog as the main set of code which can hang is complete
        PAL.WatchdogDisable();
        
        // Re-enable low-power mode, since the main async events are few and far
        // between at this point
        evm_.LowPowerEnable();
        
        // Log
        Serial.println("Sleep\n");
        PAL.Delay(20);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Message Sending
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void SendMessage()
    {
        // Kick the watchdog
        PAL.WatchdogReset();
        
        AX25UIMessage &msg = *amt_.GetAX25UIMessage();

        msg.SetDstAddress("APZ001", 0);
        msg.SetSrcAddress(userConfig_.aprs.callsign, 11);
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
            aprm.SetCommentDeviceId(userConfig_.device.id);
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
        amt_.SetTransmitCount(userConfig_.radio.transmitCount);
        amt_.SetDelayMsBetweenTransmits(userConfig_.radio.delayMsBetweenTransmits);

        // Kick the watchdog before actual transmission
        PAL.WatchdogReset();
        amt_.Transmit();
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // GPS Controls
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void StartGPS()
    {
        // enable power supply to GPS
        PAL.DigitalWrite(cfg_.pinGpsEnable, HIGH);
        
        // re-init to begin cycle again
        gps_.Init();
        
        // assert this is a high-altitude mode
        gps_.SetHighAltitudeMode();
    }
    
    void StopGPS()
    {
        // Don't allow prior fix to be used, we want a brand new fix to be
        // acquired next time the GPS starts up
        gps_.ResetFix();
        
        // stop interrups from firing in underlying code
        gps_.DisableSerialInput();
        
        // cause the gps module to store the metadata is has learned from
        // the satellites it can see and used to get a lock.
        // this will be read again automatically by the module on startup.
        gps_.SaveConfiguration();
        
        // disable power supply to GPS
        // (battery backup for module-stored data supplied through other pin)
        PAL.DigitalWrite(cfg_.pinGpsEnable, LOW);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Stats Keeping
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void StatsIncr(uint16_t &counter)
    {
        persistentCountersAccessor_.Read(persistentCounters_);
        
        ++counter;
        
        persistentCountersAccessor_.Write(persistentCounters_);
    }
    
    void StatsIncrNumRestarts()
    {
        StatsIncr(persistentCounters_.NUM_RESTARTS);
    }
    
    void StatsIncrNumWdtRestarts()
    {
        StatsIncr(persistentCounters_.NUM_WDT_RESTARTS);
    }
    
    void StatsIncrNumMsgsNotSent()
    {
        StatsIncr(persistentCounters_.NUM_MSGS_NOT_SENT);
    }
    

    ///////////////////////////////////////////////////////////////////////////
    //
    // TODO
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void ToDo()
    {
        // use LEDs as status
            // different schemes for config vs low-altitude status
            // disable when flying
        
        // set serial active checking frequency more appropriately
            // confirm working and won't eat power by waking up inappropriately
            // during actual flight
        
        // dump flash stored locations if found later
        
        // restore fuse setting to erase EEPROM on re-program
            // temporarily disabled for development
            
        // user-configure whether LEDs blink or not for status

    }


private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppPicoTracker1Config &cfg_;
    
    AppPicoTracker1UserConfigManager::UserConfig userConfig_;
    
    TimedEventHandlerDelegate tedWakeAndEvaluateTimeout_;
    TimedEventHandlerDelegate tedWaitForGpsLock_;
    
    SensorGPSUblox               gps_;
    SensorGPSUblox::Measurement  gpsMeasurement_;
    
    RFSI4463PRO                 radio_;
    AX25UIMessageTransmitter<>  amt_;
    
    GeofenceAPRS  geofence_;
    
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




























