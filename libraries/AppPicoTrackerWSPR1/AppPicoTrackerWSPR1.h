#ifndef __APP_PICO_TRACKER_WSPR_1_H__
#define __APP_PICO_TRACKER_WSPR_1_H__


#include "PAL.h"
#include "Log.h"
#include "Evm.h"
#include "AppPicoTrackerWSPR1TestableBase.h"
#include "AppPicoTrackerWSPR1UserConfigManager.h"


class AppPicoTrackerWSPR1
: public AppPicoTrackerWSPR1TestableBase
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

    AppPicoTrackerWSPR1(const AppPicoTrackerWSPR1Config &cfg)
    : AppPicoTrackerWSPR1TestableBase(cfg)
    , solarState_(SolarState::NEED_GPS_DATA)
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
        AppPicoTrackerWSPR1TestableBase::Init();
        
        Blink(cfg_.pinLedRed,   100);
        Blink(cfg_.pinLedGreen, 100);

        
        uint8_t userConfigOk = 0;
        
        // Keep the sizable manager out of memory indefinitely by only creating
        // a temporary instance.
        //
        // At this point during startup, there is no other competition for ram.
        {
            AppPicoTrackerWSPR1UserConfigManager mgr(cfg_.pinConfigure, userConfig_);
            
            // For use in testing out different configurations
            uint8_t letDefaultApplyAutomatically = 1;
            userConfigOk = mgr.GetConfig(letDefaultApplyAutomatically);
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
        uint8_t gpsHadError = 0;
        
        Log(P("\nWake"));
        
        // We're about to use a lot of power, so turn off power-saving mode,
        // which has a lower efficiency at higher current draw.
        RegulatorPowerSaveDisable();
        
        // PreSendMessage();
        // PAL.Delay(1000);
        // PostSendMessage();
        
        // Protect against hangs, which has happened
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_8000_MS);
        
        
        /////////////////////////////////////////
        //
        // Get GPS Location Lock
        //     (if needed)
        //
        /////////////////////////////////////////
        
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
                Log(P("OK"));
                
                solarState_ = SolarState::NEED_TO_TRANSMIT;
            }
            else
            {
                gpsHadError = 1;
                
                Log(P("TIMEOUT: "), userConfig_.gps.gpsLockTimeoutMs);
                
                // for (uint8_t i = 0; i < 3; ++i)
                // {
                    // PreSendMessage();
                    // PAL.Delay(1000);
                    // PostSendMessage();
                    // PAL.Delay(1000);
                // }
                
                // Duration of time to reasonably location lock exceeded.
                // Reset the module and try again later.
                ResetAndCutBatteryBackupSubsystemGPS();
            }
            
            // StopGPS() moved, see optimization below
        }
        
        /////////////////////////////////////////
        //
        // Expire old location locks if not used
        // for a long time.
        //
        /////////////////////////////////////////
        
        // Check if gps location, which could be from a prior run, is recent
        // enough to try to get a fast time lock from.
        //
        // This duration is based on observations during testing.
        //
        // Basically the initial GPS time values are wrong by seconds when the
        // GPS module itself has (I believe) stale data from having sat around
        // for a long time.
        //
        // Under expected conditions, we will be location locking frequently
        // and therefore the 2 hours won't be an issue hit.
        //
        if (gpsLocationLockOk_)
        {
            if (PAL.Millis() - gpsLocationMeasurement_.clockTimeAtMeasurement > cfg_.gpsMaxAgeLocationLockMs)
            {
                Log(P("Loc Lock aged out: "), cfg_.gpsMaxAgeLocationLockMs);
                
                solarState_ = SolarState::NEED_GPS_DATA;
                gpsLocationLockOk_ = 0;
            }
        }
        
        
        /////////////////////////////////////////
        //
        // Keep GPS on if we're about to use it
        // right away.
        //
        /////////////////////////////////////////

        // Now decide if we turn off the GPS.
        // (it might not even be on if we skipped the step above)
        //
        // Normally we would have done so in the step above.
        //
        // However, we know we're about to try for a time lock.
        //
        // There are some GPS modules which don't produce good time locks until
        // 10-15 seconds after even a brief stoppage even though some do in
        // only a second.
        //
        // So this represents an optimization.  Deal with the worst modules.
        //
        // So, if we look ahead and know we're going to do the time lock step,
        // we choose to not turn off the GPS as part of the end of the prior
        // step.
        
        uint8_t doTimeLockStep = 
            DoThisStep(Step::GPS_TIME_LOCK_AND_SEND) &&
            InputVoltageSufficient(userConfig_.power.minMilliVoltGpsTimeLock) &&
            gpsLocationLockOk_;
        
        if (doTimeLockStep)
        {
            Log(P("GPS keep ON"));
        }
        else
        {
            Log(P("GPS OFF"));
            StopGPS();
        }
        
        
        /////////////////////////////////////////
        //
        // Get a Time lock if we want to send
        //
        /////////////////////////////////////////
        
        // Sync to 2 minute mark if GPS location acquired
        // Consider available power if solar.
        if (doTimeLockStep)
        {
            Log(P("GPS locking on 2 min"));
            
            function<void(void)>    fnBeforeAttempt = [this]() { StartGPS(); };
            function<void(void)>    fnAfterAttempt  = [this]() { StopGPS(); };
            function<uint8_t(void)> fnOkToContinue  = [this]() { return InputVoltageSufficient(userConfig_.power.minMilliVoltTransmit); };
            
            uint8_t gpsTimeLockOk =
                gps_.GetNewTimeMeasurementSynchronousTwoMinuteMarkUnderWatchdog(
                    &gpsTimeMeasurement_,
                    cfg_.gpsMaxDurationTimeLockWaitMs,
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
            
            
            /////////////////////////////////////////
            //
            // Send WSPR message
            //
            /////////////////////////////////////////

            if (gpsTimeLockOk)
            {
                // If time locked, prepare to transmit.
                // Consider available power if solar.
                if (InputVoltageSufficient(userConfig_.power.minMilliVoltTransmit))
                {
                    Log('1');
                    PreSendMessage();
                    
                    Log('2');
                    // Pack message now that we know where we are
                    FillOutStandardWSPRMessage();
                    
                    // Test message before sending (but send regardless)
                    Log(P("Message prepared"));
                    
                    // Figure out how long we've been operating since the mark
                    uint32_t timeDiff = PAL.Millis() - timeAtMark;
                    
                    Log(timeDiff);
                    
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
                gpsHadError = 1;
                
                // Underlying lock code relies on the fact that the GPS should
                // get a relatively quick (seconds) time lock due to the GPS
                // having previously got a location lock, seeding the GPS with
                // enough information to accurately time lock quickly.
                //
                // If we're unable to lock, something is wrong with that
                // assumption.
                //
                // Reset and discard location lock, then sleep.
                // This will lead to re-obtaining the location lock and attempt
                // our subsequent attempt to time sync again will be again under
                // the assumed state while executing this code.
                ResetAndCutBatteryBackupSubsystemGPS();
                
                solarState_ = SolarState::NEED_GPS_DATA;
                gpsLocationLockOk_ = 0;
            }
        }
        
        
        /////////////////////////////////////////
        //
        // Sleep for however long
        //
        /////////////////////////////////////////

        // Schedule next wakeup
        uint32_t wakeAndEvaluateDelayMs = CalculateWakeup(gpsHadError);
        wakeAndEvaluateDelayMs = 0;
        tedWake_.RegisterForTimedEvent(wakeAndEvaluateDelayMs);
        
        Log(P("Sleep "), wakeAndEvaluateDelayMs);
        
        // Disable watchdog as the main set of code which can hang is complete
        PAL.WatchdogDisable();
        
        // PAL.Delay(2000);
        // for (uint8_t i = 0; i < 2; ++i)
        // {
            // PreSendMessage();
            // PAL.Delay(1000);
            // PostSendMessage();
            // PAL.Delay(1000);
        // }
        // PAL.Delay(2000);
        
        // We're about to sleep and use very little power, so turn on
        // power-saving mode, which has a higher efficiency at lower current
        // draw.
        RegulatorPowerSaveEnable();
    }
    
    uint32_t CalculateWakeup(uint8_t gpsHadError)
    {
        uint32_t retVal = 0;
        
        /*
         * If there was an error, it was the GPS, and we've reset it, so we
         * want to give it a bit of time to recover itself.
         *   This is a configured duration
         * 
         * If no error
         *   Solar has its own wakeup interval, use that.
         *   Battery has altitude-sensitive configuration, apply that.
         * 
         */
        
        if (gpsHadError)
        {
            retVal = cfg_.gpsDurationWaitPostResetMs;
        }
        else
        {
            if (userConfig_.power.solarMode)
            {
                retVal = cfg_.intervalSolarWakeupMs;
            }
            else    // battery
            {
                // consider whether still in the initial launch period where the
                // rate is sticky to low altitude configuration
                if (inLowAltitudeStickyPeriod_)
                {
                    if (PAL.Millis() < userConfig_.geo.lowAltitude.stickyMs)
                    {
                        // nothing to do, this remains true
                    }
                    else
                    {
                        // been too long, shut off and never re-activate
                        // (prevents re-activation at time wraparound)
                        inLowAltitudeStickyPeriod_ = 0;
                    }
                }
                
                // Apply altitude and sticky parameters
                if (gpsLocationMeasurement_.altitudeFt < userConfig_.geo.lowHighAltitudeFtThreshold ||
                    inLowAltitudeStickyPeriod_)
                {
                    // Wake again at the interval configured for low altitude
                    retVal = userConfig_.geo.lowAltitude.wakeAndEvaluateMs;
                }
                else    // high altitude
                {
                    // Wake again at the interval configured for high altitude active zones
                    retVal = userConfig_.geo.highAltitude.wakeAndEvaluateMs;
                }
            }
        }
        
        return retVal;
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
            uint16_t inputMilliVolt = GetInputMilliVoltage();
            
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
        
        // design around testing board and application
        
        // bring in design observations from prior boards
                
        
        test the ADC correctness after deep sleep
        
        
        
        test different message types and protocol adherence.
            can grid tolerate all caps?
                do I care?
            what about power?  can I use any value and still get packets returned to me?
            how else can I get extra data into the protocol?
        
*/
        
    }
    
    
    
    
    
    

private:

    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
    SolarState  solarState_;


    
};


#endif  // __APP_PICO_TRACKER_WSPR_1_H__




























