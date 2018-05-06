#ifndef __APP_PICO_TRACKER_WSPR_1_H__
#define __APP_PICO_TRACKER_WSPR_1_H__


#include "PAL.h"
#include "Log.h"
#include "Eeprom.h"
#include "Evm.h"
#include "SensorGPSUblox.h"
#include "WSPRMessageTransmitter.h"
#include "AppPicoTrackerWSPR1UserConfigManager.h"


struct AppPicoTrackerWSPR1Config
{
    // GPS
    uint8_t pinGpsEnable;
    uint8_t pinGpsSerialRx; // receive GPS data from this pin
    uint8_t pinGpsSerialTx; // send data to the GPS on this pin
    
    // WSPR TX
    uint8_t pinWsprTxEnable;
    
    // Status LEDs
    uint8_t pinLedRed;
    uint8_t pinLedGreen;
};


class AppPicoTrackerWSPR1
{
private:
    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
public:
    AppPicoTrackerWSPR1(AppPicoTrackerWSPR1Config &cfg)
    : cfg_(cfg)
    , gps_(cfg_.pinGpsSerialRx, cfg_.pinGpsSerialTx)
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
        LogStart(9600);
        Log(P("Starting"));
        if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_WATCHDOG)
        {
            Log(P("WDTR"));
        }
        
        // Shut down subsystems
        // Floating pins have been seen to be enough to be high enough to enable
        
        // GPS
        PAL.PinMode(cfg_.pinGpsEnable, OUTPUT);
        PAL.DigitalWrite(cfg_.pinGpsEnable, LOW);
        
        // WSPR
        PAL.PinMode(cfg_.pinWsprTxEnable, OUTPUT);
        PAL.DigitalWrite(cfg_.pinWsprTxEnable, LOW);
        
        // Set up LED pins as output
        PAL.PinMode(cfg_.pinLedRed,   OUTPUT);
        PAL.PinMode(cfg_.pinLedGreen, OUTPUT);
        
        // Blink to indicate power on
        Blink(cfg_.pinLedRed,   100);
        Blink(cfg_.pinLedGreen, 100);

        // Get user config
        if (AppPicoTrackerWSPR1UserConfigManager::GetUserConfig(userConfig_))
        {
            // Blink to indicate good configuration
            for (uint8_t i = 0; i < 3; ++i)
            {
                Blink(cfg_.pinLedGreen, 300);
            }
            
            // Maintain counters about restarts.
            // Also causes stats to be pulled from EEPROM into SRAM unconditionally.
            StatsIncrNumRestarts();
            if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_WATCHDOG)
            {
                StatsIncrNumWdtRestarts();
            }
            
            
            
            // is this comment true anymore?  haven't worked out tx schedule yet...
            
            
            // Begin tracker reporting, fire first event immediately.
            // No point using interval, the decision about how long to sleep for is
            // evaluated each time.
            tedWakeAndEvaluateTimeout_.SetCallback([this](){
                OnWakeAndEvaluateTimeout();
            });
            tedWakeAndEvaluateTimeout_.RegisterForTimedEvent(0);
            
            // Handle async events
            Log(P("Running"));
            
            evm_.MainLoopLowPower();
        }
        else
        {
            // Blink to indicate bad configuration
            while (1)
            {
                Blink(cfg_.pinLedRed, 300);
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

    ///////////////////////////////////////////////////////////////////////////
    //
    // Main Wake/Lock/Send logic
    //
    ///////////////////////////////////////////////////////////////////////////

    void OnWakeAndEvaluateTimeout()
    {
        // Log
        Log("\nWake");

        // Begin monitoring code which has been seen to hang
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_8000_MS);
        
        // No need for low-power mode while we're attempting to do the primary
        // tracker behavior of lock and send.
        // Besides, we're going to async-spin waiting for a GPS lock, and in the
        // meantime we can use a watchdog
        evm_.LowPowerDisable();
        
        // Start GPS
        Log("GPS ON");
        StartGPS();
        
        // Warm up transmitter
        PrepareToSendMessage();
        
        // Lock onto two-minute mark on GPS time
        uint8_t gpsLockOk = WaitForNextGPSTwoMinuteMark();
        
        if (gpsLockOk)
        {
            // We're going to transmit at the :01 second mark.
            // Do some useful work in the meantime, but keep track of how much
            // time it takes.
            uint32_t timeStart = PAL.Millis();
            
            // Pack message now that we know where we are
            FillOutStandardWSPRMessage();
            
            // Figure out how long that took
            uint32_t timeEnd = PAL.Millis();
            uint32_t timeDiff = timeEnd - timeStart;
            
            // Wait for the :01 mark
            PAL.Delay(1000L - timeDiff);
            Log(P("TX"));
            
            // Actually send WSPR
            SendMessage();

            // Debug
            Log(P("    Fill out took: "), timeDiff);
        }
        else
        {
            // keep a stat?
        }
        
        
        
        
        
        
        
        // Wait for remaining time, continuously sync'ing against GPS
        
        
        
        // Now burn off final 1 second
        
        
        
        
        
        // keep stat about bad gps locks
        
        
        // unconditionally turn off transmitter
        // unconditionally turn off gps
        
        
        
        // Disable watchdog as the main set of code which can hang is complete
        PAL.WatchdogDisable();
        
        // Re-enable low-power mode, since the main async events are few and far
        // between at this point
        evm_.LowPowerEnable();
        
        // Log
        LogNNL("Sleep ", wakeAndEvaluateMs);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Message Sending
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void PrepareToSendMessage()
    {
        // Enable power to subsystem
        PAL.DigitalWrite(cfg_.pinWsprTxEnable, HIGH);
        
        // Set up calibration details
        WSPRMessageTransmitter::Calibration c;
        
        c.systemClockOffsetMs = userConfig_.systemClockOffsetMs;
        
        // Configure transmitter with calibration details
        wsprMessageTransmitter_.SetCalibration(c);
        
        // Prepare system for send, warm up internals
        wsprMessageTransmitter_.RadioOn();
    }
    
    void FillOutStandardWSPRMessage()
    {
        wsprMessage_.SetCallsign(userConfig_.callsign);
        wsprMessage_.SetGrid();
        wsprMessage_.SetPower();            
    }
    
    void SendMessage()
    {
        // Kick the watchdog
        PAL.WatchdogReset();
        
        // Set up the transmitter to kick the watchdog also
        wsprMessageTransmitter_.SetCallbackOnBitChange([](){
            PAL.WatchdogReset();
        });
   
        // Send the message synchronously
        wsprMessageTransmitter_.Send(&wsprMessage_);
        
        // Go back to idle state
        wsprMessageTransmitter_.RadioOff();
        
        // Cut power to subsystem
        PAL.DigitalWrite(cfg_.pinWsprTxEnable, LOW);
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
    
    void StatsIncr(uint16_t &counter, uint16_t incrBy = 1)
    {
        persistentCountersAccessor_.Read(persistentCounters_);
        
        counter += incrBy;
        
        persistentCountersAccessor_.Write(persistentCounters_);
    }
    
    void StatsIncrSeqNo()
    {
        StatsIncr(persistentCounters_.SEQ_NO);
    }
    
    void StatsIncrGpsLockWaitSecs(uint16_t incrBy)
    {
        StatsIncr(persistentCounters_.GPS_LOCK_WAIT_SECS, incrBy);
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
        // control whether regulator is in power save mode or not
        
        // design around testing board and application
        
        // bring in design observations from prior boards
        
        // give up on gps lock after a timeout
        
        // hide WSPR tx and message making behind other classes
        
        // how to handle clock speed of device differing between chips?
            // calibration interface?
            
        // which counters to keep?
    }
    
    
    
    
    
    

private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppPicoTrackerWSPR1Config &cfg_;
    
    AppPicoTrackerWSPR1UserConfigManager::UserConfig userConfig_;
    
    TimedEventHandlerDelegate tedWakeAndEvaluateTimeout_;
    
    SensorGPSUblox               gps_;
    SensorGPSUblox::Measurement  gpsMeasurement_;
    
    WSPRMessage             wsprMessage_;
    WSPRMessageTransmitter  wsprMessageTransmitter_;
    
    uint8_t inLowAltitudeStickyPeriod_ = 1;
    
    struct PersistentCounters
    {
        uint16_t SEQ_NO             = 0;
        uint16_t GPS_LOCK_WAIT_SECS = 0;
        uint16_t NUM_RESTARTS       = 0;
        uint16_t NUM_WDT_RESTARTS   = 0;
        uint16_t NUM_MSGS_NOT_SENT  = 0;
    };
    
    PersistentCounters                 persistentCounters_;
    EepromAccessor<PersistentCounters> persistentCountersAccessor_;
};


#endif  // __APP_PICO_TRACKER_WSPR_1_H__




























