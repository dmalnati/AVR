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
        // Floating pins have been seen to be enough to be high enough to
        // cause unintentional operation
        
        // GPS Subsystem
        PAL.PinMode(cfg_.pinGpsEnable, OUTPUT);
        StopSubsystemGPS();
        
        // WSPR Subsystem
        PAL.PinMode(cfg_.pinWsprTxEnable, OUTPUT);
        StopSubsystemWSPR();
        
        // Set up LEDs and blink to indicate power up
        PAL.PinMode(cfg_.pinLedRed,   OUTPUT);
        PAL.PinMode(cfg_.pinLedGreen, OUTPUT);
        
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
        Log(P("\nWake"));

        // Begin monitoring code which has been seen to hang
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_8000_MS);
        
        // Start GPS
        Log(P("GPS ON"));
        StartGPS();
        
        // Warm up transmitter
        Log(P("Warming transmitter"));
        PrepareToSendMessage();
        
        // Lock onto two-minute mark on GPS time
        Log(P("GPS locking to next 2 minute mark"));
        uint8_t gpsLockOk = gps_.WaitForNextGPSTwoMinuteMark(&gpsMeasurement_);

        // The GPS may have locked at an even minute and :00 seconds.
        // We're going to transmit at the :01 second mark.
        //
        // Do some useful work in the meantime, but keep track of how much
        // time it takes so we can wake up on time.
        //
        // We also account for the fact that the genuine real-world time change
        // occurred in the past, and we're only learning about it after
        // having processed the output data from the GPS, which is relatively
        // slow at 9600 baud.
        uint32_t timeAtMark =
            PAL.Millis() - SensorGPSUblox::MIN_DELAY_NEW_TIME_LOCK_MS;
        
        Log(P("GPS Lock "), gpsLockOk ? P("OK") : P("NOT OK"));
        
        // Unconditionally turn off GPS, we have what we need one way or another
        Log(P("GPS OFF"));
        StopGPS();
        
        // If locked, prepare to transmit
        if (gpsLockOk)
        {
            // Pack message now that we know where we are
            uint8_t messageOk = FillOutStandardWSPRMessage();
            
            // Test message before sending (but send regardless)
            Log(P("Message prepared, "), messageOk ? P("OK") : P("NOT OK"));
            
            // Figure out how long we've been operating since the mark
            uint32_t timeDiff = PAL.Millis() - timeAtMark;
            
            // Wait for the :01 second mark after even minute, accounting for 
            // time which has elapsed since the even minute
            const uint32_t ONE_SECOND = 1000UL;
            if (timeDiff < ONE_SECOND)
            {
                PAL.Delay(ONE_SECOND - timeDiff);
            }
            
            // Send WSPR message
            Log(P("TX"));
            SendMessage();
        }
        
        // Schedule next wakeup
        uint32_t wakeAndEvaluateDelayMs = 0;
        tedWakeAndEvaluateTimeout_.RegisterForTimedEvent(wakeAndEvaluateDelayMs);
        
        // Disable watchdog as the main set of code which can hang is complete
        PAL.WatchdogDisable();
        
        Log(P("Sleep "), wakeAndEvaluateDelayMs);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // WSPR Transmitter Control
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void StartSubsystemWSPR()
    {
        PAL.DigitalWrite(cfg_.pinWsprTxEnable, HIGH);
        
        // Give device time to start up.  Value found empirically.
        PAL.Delay(50);
    }
    
    void StopSubsystemWSPR()
    {
        PAL.DigitalWrite(cfg_.pinWsprTxEnable, LOW);
    }
    
    void PrepareToSendMessage()
    {
        // Enable subsystem
        StartSubsystemWSPR();
        
        // Configure transmitter with calibration details
        // Debug
        userConfig_.mtCalibration.crystalCorrectionFactor = 0;
        userConfig_.mtCalibration.systemClockOffsetMs     = 8;
        wsprMessageTransmitter_.SetCalibration(userConfig_.mtCalibration);
        
        // Set up the transmitter to kick the watchdog when sending data later
        wsprMessageTransmitter_.SetCallbackOnBitChange([](){
            PAL.WatchdogReset();
        });
        
        // Prepare system for send, warm up internals
        wsprMessageTransmitter_.RadioOn();
    }

    uint8_t SendMessage()
    {
        uint8_t retVal = 0;
        
        // Kick the watchdog
        PAL.WatchdogReset();
        
        // Send the message synchronously
        retVal = wsprMessageTransmitter_.Send(&wsprMessage_);
        
        // Go back to idle state
        wsprMessageTransmitter_.RadioOff();
        
        // Disable subsystem
        StopSubsystemWSPR();
        
        return retVal;
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // WSPR Message Construction
    //
    ///////////////////////////////////////////////////////////////////////////
    
    uint8_t FillOutStandardWSPRMessage()
    {
        // Modify the GPS maidenheadGrid to be 4 char instead of 6
        gpsMeasurement_.maidenheadGrid[4] = '\0';
        
        // Keep a mapping of altitude to power level as an encoding
        struct
        {
            uint32_t altitudeFt;
            uint8_t  powerDbm;
        } altitudeToPowerList[] = {
            {     0,   0 },
            {  2222,   3 },
            {  4444,   7 },
            {  6667,  10 },
            {  8889,  13 },
            { 11111,  17 },
            { 13333,  20 },
            { 15556,  23 },
            { 17778,  27 },
            { 20000,  30 },
            { 22222,  33 },
            { 24444,  37 },
            { 26667,  40 },
            { 28889,  43 },
            { 31111,  47 },
            { 33333,  50 },
            { 35556,  53 },
            { 37778,  57 },
            { 40000,  60 },
        };
        
        // Default to lowest altitude, and progressively look for altitudes that
        // we are gte to.
        uint8_t powerDbm = altitudeToPowerList[0].powerDbm;
        for (auto altToPwr : altitudeToPowerList)
        {
            if (gpsMeasurement_.altitudeFt >= altToPwr.altitudeFt)
            {
                powerDbm = altToPwr.powerDbm;
            }
        }
        
        // Fill out actual message
        //wsprMessage_.SetCallsign((const char *)userConfig_.callsign);
        // temporary workaround while waiting to implement this
        wsprMessage_.SetCallsign("KD2KDD");
        wsprMessage_.SetGrid(gpsMeasurement_.maidenheadGrid);
        // debug
        //wsprMessage_.SetGrid("AB12");
        wsprMessage_.SetPower(powerDbm);
        
        return wsprMessageTransmitter_.Test(&wsprMessage_);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // GPS Controls
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void StartSubsystemGPS()
    {
        gps_.EnableSerialInput();
        
        PAL.DigitalWrite(cfg_.pinGpsEnable, HIGH);

        gps_.EnableSerialOutput();
    }
    
    void StopSubsystemGPS()
    {
        // Work around a hardware issue where the GPS will draw lots of current
        // through a logic input
        gps_.DisableSerialOutput();
        
        // disable power supply to GPS
        // (battery backup for module-stored data supplied through other pin)
        PAL.DigitalWrite(cfg_.pinGpsEnable, LOW);
        
        // stop interrupts from firing in underlying code
        gps_.DisableSerialInput();
    }
    
    void StartGPS()
    {
        // Enable subsystem
        StartSubsystemGPS();
        
        // re-init to begin cycle again
        gps_.Init();
        
        // assert this is a high-altitude mode
        gps_.SetHighAltitudeMode();
        
        // Only care about two messages, indicate so
        gps_.EnableOnlyGGAAndRMC();
    }

    void StopGPS()
    {
        // Don't allow prior fix to be used, we want a brand new fix to be
        // acquired next time the GPS starts up
        gps_.ResetFix();
        
        // cause the gps module to store the metadata is has learned from
        // the satellites it can see and used to get a lock.
        // this will be read again automatically by the module on startup.
        gps_.SaveConfiguration();

        // Disable subsystem
        StopSubsystemGPS();
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
                
        // how to handle clock speed of device differing between chips?
            // calibration interface?
            
        // which counters to keep?
        
        // print out time when locked on
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




























