#ifndef __APP_PICO_TRACKER_WSPR_1_H__
#define __APP_PICO_TRACKER_WSPR_1_H__


#include "PAL.h"
#include "Log.h"
#include "Evm.h"
#include "SensorGPSUblox.h"
#include "WSPRMessageTransmitter.h"
#include "AppPicoTrackerWSPR1UserConfigManager.h"


struct AppPicoTrackerWSPR1Config
{
    // Human interfacing
    uint8_t pinConfigure;
    
    // Pre-regulator power sensing
    uint8_t pinInputVoltage;
    
    // Regulator control
    uint8_t pinRegPowerSaveEnable;
    
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

    enum class Step : uint8_t
    {
        GPS_LOCATION_LOCK,
        GPS_TIME_LOCK_AND_SEND,
    };

    enum class SolarState : uint8_t
    {
        NEED_GPS_DATA,
        NEED_TO_TRANSMIT,
    };

    
public:
    AppPicoTrackerWSPR1(AppPicoTrackerWSPR1Config &cfg)
    : cfg_(cfg)
    , solarState_(SolarState::NEED_GPS_DATA)
    , gps_(cfg_.pinGpsSerialRx, cfg_.pinGpsSerialTx)
    , gpsLocationLockOk_(0)
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
        LogNL();
        Log(P("Starting"));
        
        if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_WATCHDOG)
        {
            Log(P("WDTR"));
        }
        else if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_BROWNOUT)
        {
            Log(P("BODR"));
        }
        
        // Don't allow being too low of a BOD.
        if (PAL.GetFuseBODLimMilliVolts() != 2700)
        {
            Log(P("BOD WRONG -- Set to 2700"));
            PAL.SoftReset();
        }
        
        // Set up control over regulator power save mode.
        PAL.PinMode(cfg_.pinRegPowerSaveEnable, OUTPUT);
        RegulatorPowerSaveDisable();
        
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

        
        uint8_t userConfigOk = 0;
        
        // Keep the sizable manager out of memory indefinitely by only creating
        // a temporary instance.
        //
        // At this point during startup, there is no other competition for ram.
        {
            AppPicoTrackerWSPR1UserConfigManager mgr(cfg_.pinConfigure, userConfig_);
            
            userConfigOk = mgr.GetConfig();
        }
        
        if (userConfigOk)
        {
            // Blink to indicate good configuration
            for (uint8_t i = 0; i < 3; ++i)
            {
                Blink(cfg_.pinLedGreen, 300);
            }
            
            // Begin tracker reporting, fire first event immediately.
            // No point using interval, the decision about how long to sleep for is
            // evaluated each time.
            tedWake_.SetCallback([this](){
                OnWake();
            });
            tedWake_.RegisterForTimedEvent(0);
            
            // Going into low-current sleep, enable power save
            RegulatorPowerSaveEnable();
            
            // Handle async events
            Log(P("Running"));
            evm_.MainLoopLowPower();
        }
        else
        {
            Log(P("Bad Configuration"));
            
            // Blink to indicate bad configuration
            while (1)
            {
                Blink(cfg_.pinLedRed, 300);
            }
        }
    }
    
private:

    ///////////////////////////////////////////////////////////////////////////
    //
    // Main Wake/Lock/Send logic
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void OnWake()
    {
        Log(P("\nWake"));
        
        // We're about to use a lot of power, so turn off power-saving mode,
        // which has a lower efficiency at higher current draw.
        RegulatorPowerSaveDisable();
        
        // Protect against hangs, which has happened
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_8000_MS);
        
        // Get GPS location lock if one isn't already available from a prior run.
        // Consider available power if solar.
        if (DoThisStep(Step::GPS_LOCATION_LOCK) &&
            InputVoltageSufficient(userConfig_.power.minMilliVoltGpsLocationLock))
        {
            // Start GPS
            Log(P("GPS ON"));
            StartGPS();
            
            // Attempt to get lock, but time out if taking too long
            gpsLocationLockOk_ =
                gps_.GetNewLocationMeasurementSynchronousUnderWatchdog(
                    &gpsLocationMeasurement_,
                     userConfig_.gps.gpsLockTimeoutMs);
        
            if (gpsLocationLockOk_)
            {
                solarState_ = SolarState::NEED_TO_TRANSMIT;
            }
            else
            {
                // Underlying lock code relies on the fact that the GPS should
                // get a relatively quick (seconds) time lock due to the GPS
                // having previously got a location lock, seeding the GPS with
                // enough information to time lock quickly.
                //
                // If we're unable to lock, something is wrong with that
                // assumption.
                //
                // Reset and discard location lock, then sleep.
                // Re-obtain location lock and attempt to sync again, thereby
                // restoring the assumed state while executing this code.
                gps_.ResetModule();
            }
            
            // Stop GPS
            Log(P("GPS OFF"));
            StopGPS();
        }
        
        
        // Check if gps location, which could be from a prior run, is recent
        // enough to try to get a fast time lock from.
        // This duration is based on observations during testing.
        // We choose 2 hours old
        if (gpsLocationLockOk_)
        {
            const uint32_t MAX_AGE_LOCATION_LOCK_MS = 2UL * 60UL * 60UL * 1000UL;
            if (PAL.Millis() - gpsLocationMeasurement_.clockTimeAtMeasurement > MAX_AGE_LOCATION_LOCK_MS)
            {
                solarState_ = SolarState::NEED_GPS_DATA;
                gpsLocationLockOk_ = 0;
            }
        }
        
        
        // Sync to 2 minute mark if GPS location acquired
        // Consider available power if solar.
        if (DoThisStep(Step::GPS_TIME_LOCK_AND_SEND) &&
            InputVoltageSufficient(userConfig_.power.minMilliVoltGpsTimeLock) &&
            gpsLocationLockOk_)
        {
            Log(P("GPS locking 2 min"));
            
            const uint32_t DURATION_MAX_GPS_TIME_LOCK_WAIT_MS = 5000;

            function<void(void)>    fnBeforeAttempt = [this]() { StartGPS(); };
            function<void(void)>    fnAfterAttempt  = [this]() { StopGPS(); };
            function<uint8_t(void)> fnOkToContinue  = [this]() { return InputVoltageSufficient(userConfig_.power.minMilliVoltTransmit); };
            
            uint8_t gpsTimeLockOk =
                gps_.GetNewTimeMeasurementSynchronousTwoMinuteMarkUnderWatchdog(
                    &gpsTimeMeasurement_,
                    DURATION_MAX_GPS_TIME_LOCK_WAIT_MS,
                    fnBeforeAttempt,
                    fnAfterAttempt,
                    fnOkToContinue
                );

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
            
            Log(gpsTimeLockOk ? P("OK") : P("NOT OK"));
            
            if (gpsTimeLockOk)
            {
                // If time locked, prepare to transmit.
                // Consider available power if solar.
                if (InputVoltageSufficient(userConfig_.power.minMilliVoltTransmit))
                {
                    PreSendMessage();
                    
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
                    
                    PostSendMessage();
                    
                    // Change solar state and declare gps lock no longer good
                    solarState_ = SolarState::NEED_GPS_DATA;
                    gpsLocationLockOk_ = 0;
                }
            }
            else
            {
                // Assumption is this time lock should work if the location lock
                // was any good from recently enough.
                // Clearly we didn't lock, and so we need to get back to the
                // assumed state of having a good recent location lock.
                //
                // Therefore we throw away the location lock.
                solarState_ = SolarState::NEED_GPS_DATA;
                gpsLocationLockOk_ = 0;
            }
        }
        
        // Disable watchdog as the main set of code which can hang is complete
        PAL.WatchdogDisable();
        
        // We're about to sleep and use very little power, so turn on
        // power-saving mode, which has a higher efficiency at lower current
        // draw.
        RegulatorPowerSaveEnable();
        
        // Schedule next wakeup
        uint32_t wakeAndEvaluateDelayMs = 0;
        tedWake_.RegisterForTimedEvent(wakeAndEvaluateDelayMs);
        
        Log(P("Sleep "), wakeAndEvaluateDelayMs);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Solar and State Interpretation
    //
    ///////////////////////////////////////////////////////////////////////////
    
    // Convenience function to wrap up the policy about what steps to do and
    // in what order, depending on solar or battery.
    //
    // The rules are:
    // - If battery powered, do every step in order
    // - If solar powered
    //   - depends on the state you came in at, which represents the fact that
    //     you may have a GPS location lock that you haven't transmitted yet
    //     from a prior run.
    //
    uint8_t DoThisStep(Step step)
    {
        uint8_t retVal = 0;
        
        if (userConfig_.power.solarMode)
        {
            if (step == Step::GPS_LOCATION_LOCK)
            {
                retVal = (solarState_ == SolarState::NEED_GPS_DATA);
            }
            else if (step == Step::GPS_TIME_LOCK_AND_SEND)
            {
                retVal = (solarState_ == SolarState::NEED_TO_TRANSMIT);
            }
        }
        else    // battery mode
        {
            retVal = 1;
        }
        
        return retVal;
    }
    
    // Abstract away the differences between battery and solar when considering
    // whether a particular step has sufficient power to proceed.
    uint8_t InputVoltageSufficient(uint16_t solarMilliVoltMinimum)
    {
        uint8_t retVal = 0;
        
        if (userConfig_.power.solarMode)
        {
            // Circuit has a voltage divider halve the input voltage.
            uint16_t inputMilliVolt = PAL.AnalogRead(cfg_.pinInputVoltage) * 2;
            
            if (inputMilliVolt >= solarMilliVoltMinimum)
            {
                retVal = 1;
            }
        }
        else    // battery mode
        {
            retVal = 1;
        }
        
        return retVal;
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // WSPR Transmitter Control
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void StartSubsystemWSPR()
    {
        PAL.DigitalWrite(cfg_.pinWsprTxEnable, HIGH);
    }
    
    void StopSubsystemWSPR()
    {
        PAL.DigitalWrite(cfg_.pinWsprTxEnable, LOW);
    }
    
    void PreSendMessage()
    {
        // Enable subsystem
        StartSubsystemWSPR();
        
        // Configure transmitter with calibration details
        // Debug
        userConfig_.radio.mtCalibration.crystalCorrectionFactor = 0;
        userConfig_.radio.mtCalibration.systemClockOffsetMs     = 8;
        wsprMessageTransmitter_.SetCalibration(userConfig_.radio.mtCalibration);
        
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
        
        return retVal;
    }
    
    void PostSendMessage()
    {
        // Go back to idle state
        wsprMessageTransmitter_.RadioOff();
        
        // Disable subsystem
        StopSubsystemWSPR();
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // WSPR Message Construction
    //
    ///////////////////////////////////////////////////////////////////////////
    
    uint8_t FillOutStandardWSPRMessage()
    {
        // Modify the GPS maidenheadGrid to be 4 char instead of 6
        gpsLocationMeasurement_.maidenheadGrid[4] = '\0';
        
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
            if (gpsLocationMeasurement_.altitudeFt >= altToPwr.altitudeFt)
            {
                powerDbm = altToPwr.powerDbm;
            }
        }
        
        // Fill out actual message
        //wsprMessage_.SetCallsign((const char *)userConfig_.callsign);
        // temporary workaround while waiting to implement this
        wsprMessage_.SetCallsign("KD2KDD");
        wsprMessage_.SetGrid(gpsLocationMeasurement_.maidenheadGrid);
        // debug
        //wsprMessage_.SetGrid("AB12");
        wsprMessage_.SetPower(powerDbm);
        
        return wsprMessageTransmitter_.Test(&wsprMessage_);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Power Regulator Controls
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void RegulatorPowerSaveEnable()
    {
        PAL.DigitalWrite(cfg_.pinRegPowerSaveEnable, LOW);
    }
    
    void RegulatorPowerSaveDisable()
    {
        PAL.DigitalWrite(cfg_.pinRegPowerSaveEnable, HIGH);
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
    // Misc
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void Blink(uint8_t pin, uint32_t durationMs)
    {
        PAL.DigitalWrite(pin, HIGH);
        PAL.Delay(durationMs);
        PAL.DigitalWrite(pin, LOW);
        PAL.Delay(durationMs);
    }

    
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // TODO
    //
    ///////////////////////////////////////////////////////////////////////////

    void ToDo()
    {
        
        
/*
        
        // control whether regulator is in power save mode or not
        
        // design around testing board and application
        
        // bring in design observations from prior boards
                
        // how to handle clock speed of device differing between chips?
            // calibration interface?
            
        // which counters to keep?
        
        // print out time when locked on
        
        
        
        No BOD lower voltage allowed
        
        Change pin signalling configuration away from being serial in
        
        
                
        GPS giveup timeout
        GPS time sync decoupled from lock
        Care about voltage levels of input for operating certain events
        Solar vs battery support
            Tracker can be in different states depending whether GPS locked last run
                No information persists across reboots
                
        
        
        
        sticky only applies in battery mode?
        
        voltages only apply in solar mode
        
        high/low alt applies generally
        
*/
        
    }
    
    
    
    
    
    

private:

    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppPicoTrackerWSPR1Config &cfg_;

    AppPicoTrackerWSPR1UserConfig userConfig_;
    
    SolarState  solarState_;
    
    TimedEventHandlerDelegate tedWake_;
    
    SensorGPSUblox               gps_;
    SensorGPSUblox::Measurement  gpsLocationMeasurement_;
    uint8_t                      gpsLocationLockOk_;
    SensorGPSUblox::Measurement  gpsTimeMeasurement_;
    
    WSPRMessage             wsprMessage_;
    WSPRMessageTransmitter  wsprMessageTransmitter_;
};


#endif  // __APP_PICO_TRACKER_WSPR_1_H__




























