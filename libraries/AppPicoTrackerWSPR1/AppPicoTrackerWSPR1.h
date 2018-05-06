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
    
    uint8_t GetGPSLockUnderWatchdog(uint32_t timeoutMs)
    {
        const uint32_t ONE_SECOND_MS = 1000L;
        
        uint8_t retVal = 0;
        
        uint8_t cont = 1;
        while (cont)
        {
            // Kick the watchdog
            PAL.WatchdogReset();
            
            // Calculate how long to wait for, one second or less each time
            uint32_t timeoutMsGps = timeoutMs > ONE_SECOND_MS ? ONE_SECOND_MS : timeoutMs;
            
            // Attempt lock
            retVal = gps_.GetNewMeasurementSynchronous(&gpsMeasurement_, timeoutMsGps)
            
            if (retVal)
            {
                cont = 0;
            }
            else
            {
                durationRemaining -= timeoutMsGps;
                
                if (durationRemaining == 0)
                {
                    cont = 0;
                }
            }
        }
        
        return retVal;
    }
    
    uint8_t WaitForNextGPSTwoMinuteMark()
    {
        const uint32_t TWO_MINUTES_MS = 2 * 60 * 1000L;
        
        // Initially we want to get a fresh lock since there is no guaranteed
        // prior state.  We don't care about the 2 minute mark yet.  We just
        // want the known current state.
        uint8_t gpsLockOk = GetGPSLockUnderWatchdog(TWO_MINUTES_MS);
        
        if (gpsLockOk)
        {
            // Determine the digits on the clock of the next 2-minute mark
            // We know the seconds on the clock will be :00
            // The minute is going to be even.
            // So, if the current minute is:
            // - odd,  that means we're at most  60 sec away from the mark
            // - even, that means we're at most 120 sec away from the mark
            //
            // Later we deal with the case where we're already exactly at a
            // two-minute mark.
            uint8_t currentMinIsOdd = gpsMeasurement_.minute & 0x01;
            uint8_t maximumMinutesBeforeMark = currentMinIsOdd ? 1 : 2;
            
            uint8_t markMin = (gpsMeasurement_.minute + maximumMinutesBeforeMark) % 60;

            
            // Track using GPS, or determine that we're already there
            uint8_t cont = 1;
            while (cont)
            {
                // Determine how far into the future the mark time is.
                if (((gpsMeasurement_.minute + 1) % 60) == markMin)
                {
                    // We're at most 1 minute away.
                    maximumMinutesBeforeMark = 1;
                }
                else if (((gpsMeasurement_.minute + 2) % 60) == markMin)
                {
                    // We're at most 2 minutes away.
                    maximumMinutesBeforeMark = 2;
                }
                else
                {
                    // Shouldn't be possible...
                    
                    Log(P("GPS Sync Logic Busted"));
                    Log(markMin);
                    Log(gpsMeasurement_.minute);
                    Log(gpsMeasurement_.second);
                    
                    gpsLockOk = 0;
                    break;  // yikes
                }
                
                // Calculate how long until the mark.
                // Take into consideration the seconds and milliseconds on the
                // clock currently.
                //
                // eg if we're at 01:27.250
                // then
                // - markMin = 2
                // - maximumMinutesBeforeMark = 1
                // - durationBeforeMarkMs = 
                //     (60 *  1 * 1000) -     // 60,000
                //     (     27 * 1000) -     // 27,000
                //     (           250)       //    250
                // - durationBeforeMarkMs  = 32,750 // 32 sec, 750ms, correct
                // 
                uint32_t durationBeforeMarkMs =
                    (60L * maximumMinutesBeforeMark * 1000L) -
                    (gpsMeasurement_.second * 1000L) -
                    (gpsMeasurement_.millisecond);            
                
                
                // If we're at 01:59.300, then we'll be less than 1 second away
                // from the next likely lock, and so we'll just consider the
                // current lock plus a delay to be good enough.
                //
                // If we're at 02:00.000, then we will calculate that we're
                // 120,000ms away, which is correct, but also means we're at the
                // 2 min mark right now.  We detect this and break out early.
                //
                uint8_t needToTrackToMark = 1;
                if (durationBeforeMarkMs < 1000)
                {
                    // We are less than 1 second away from the mark.
                    //
                    // This isn't expected to happen much.
                    //
                    // It'd have to be the case that the GPS output a sub-second
                    // timestamp, which I've seen on a cold start, where the
                    // first valid timestamp was mid-second.
                    //
                    // However, I've only seen exact-second timestamps
                    // after the first timestamp from the GPS.
                    //
                    // This protection will remain in place to catch it anyway.
                    
                    needToTrackToMark = 0;
                    
                    PAL.Delay(durationBeforeMarkMs);
                }
                else if (durationBeforeMarkMs == 120000L)
                {
                    // We are at the 2 min mark right now.
                    //
                    // This is expected to only be possible on the first loop
                    // iteration.
                    
                    needToTrackToMark = 0;
                }
                
                // Use the GPS to lock again such that we can get an updated
                // GPS-accurate time instead of relying on our inaccurate
                // system clock to wait long periods of time.
                //
                // Need a fudge factor for GPS lock giveup time, though.
                // This is specifically important as we draw down on the 2 min
                // mark.
                //
                // Let's say we get a new lock every second, because that's the 
                // frequency of the gps module.
                //
                // When we're 5 seconds out, we specify we timeout in 5 seconds,
                // but will typically lock again in 1 second due to the
                // frequency that the GPS outputs messages.
                //
                // But let's say our giveup time is specified in ms as measured
                // by system time, and system time is running fast.
                // 
                // So our system clock, when limited to 5s, locks in 1s,
                // actually shows it took 1.001 sec.
                // No big deal in this case, because our timeout was 5 sec.
                //
                // Compare that to when there is only 1 second remaining.
                // In that case, we do lock in real-world 1 second, but our
                // system clock ran fast and expired shortly beforehand, never
                // giving us our proper GPS lock.
                //
                // This would lead to us correctly tracking toward our target
                // time with the gps, but declaring a failure at the very last
                // moment.
                //
                // To combat this, we should add a constant fudge factor to the
                // giveup time.
                //
                if (needToTrackToMark)
                {
                    const uint32_t SYSTEM_CLOCK_FAST_PROTECTION_MS = 100;
                    gpsLockOk = GetGPSLockUnderWatchdog(durationRemainingMs + SYSTEM_CLOCK_FAST_PROTECTION_MS);
                    
                    if (gpsLockOk)
                    {
                        // Check if we're at the right time.
                        // If not, we'll calculate how long wait on the next
                        // loop iteration.
                        if (gpsMeasurement_.minute == markMin)
                        {
                            // We're there.
                            // This will only happen if this loop has gone around at
                            // least once.
                            cont = 0;
                        }
                    }
                    else
                    {
                        cont = 0;
                    }
                }
                else
                {
                    // We're there.
                    cont = 0;
                }
            }
        }
        else
        {
            // Just return, bad lock
        }
        
        return gpsLockOk;
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




























