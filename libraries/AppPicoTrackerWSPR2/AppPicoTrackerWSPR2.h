#ifndef __APP_PICO_TRACKER_WSPR_2_H__
#define __APP_PICO_TRACKER_WSPR_2_H__


#include "PAL.h"
#include "Log.h"
#include "Evm.h"
#include "AppPicoTrackerWSPR2TestableBase.h"
#include "AppPicoTrackerWSPR2UserConfigManager.h"


class AppPicoTrackerWSPR2
: public AppPicoTrackerWSPR2TestableBase
{
public:

    AppPicoTrackerWSPR2(const AppPicoTrackerWSPR2Config &cfg)
    : AppPicoTrackerWSPR2TestableBase(cfg)
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
        AppPicoTrackerWSPR2TestableBase::Init();
        
        // Indicate device is on
        Blink(cfg_.pinLedRed,   100);
        Blink(cfg_.pinLedGreen, 100);

        // Indicate radio working
        Log(P("Radio Test"));
        for (uint8_t i = 0; i < 3; ++i)
        {
            PreSendMessage();
            PAL.Delay(1000);
            StopSubsystemWSPR();
            PAL.Delay(200);
        }

        // Interact with user if present
        uint8_t userConfigOk = 0;
        
        // Keep the sizable manager out of memory indefinitely by only creating
        // a temporary instance.
        //
        // At this point during startup, there is no other competition for ram.
        {
            AppPicoTrackerWSPR2UserConfigManager mgr(cfg_.pinConfigure, userConfig_);
            
            // For use in testing out different configurations
            uint8_t letDefaultApplyAutomatically = 0;
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
        
        // Protect against hangs
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_8000_MS);
        
        
        /////////////////////////////////////////
        //
        // Get Temperature
        //
        /////////////////////////////////////////
        
        StartSubsystemTemperature();
        tempC_        = sensorTemp_.GetTempC();
        int8_t tempF  = sensorTemp_.GetTempF();
        StopSubsystemTemperature();

        Log(P("Temperature "), tempF, P("F, "), tempC_, P("C"));


        /////////////////////////////////////////
        //
        // Get GPS Location Lock
        //
        /////////////////////////////////////////
        
        // Start GPS
        Log(P("GPS searching for location"));
        StartGPS();

        uint8_t gpsHadError = 0;
        
        // Attempt to get lock, but time out if taking too long
        uint8_t gpsLocationLockOk =
            gps_.GetNewLocationMeasurementSynchronousUnderWatchdog(
                &gpsLocationMeasurement_,
                cfg_.gpsMaxDurationLocationLockWaitMs
            );
    
        if (gpsLocationLockOk)
        {
            Log(P("  OK"));
        }
        else
        {
            gpsHadError = 1;

            Log(P("  Timeout: "), cfg_.gpsMaxDurationLocationLockWaitMs);
            
            // Duration of time to reasonably location lock exceeded.
            // Reset the module and try again later.
            ResetAndCutBatteryBackupSubsystemGPS();
            
            StopGPS();
        }

        /////////////////////////////////////////
        //
        // Construct messages in advance of time sync
        //
        /////////////////////////////////////////
        
        if (gpsLocationLockOk)
        {
            // Pack messages now that we know where we are
            FillOutWSPRMessageLiteral();
            FillOutWSPRMessageEncoded();

            Log(P("WSPR msgs ready"));
        }
        
        /////////////////////////////////////////
        //
        // Get a Time lock if we want to send
        //
        /////////////////////////////////////////
        
        // Sync to 2 minute mark if GPS location acquired
        if (gpsLocationLockOk)
        {
            Log(P("GPS synchronizing to 2 min mark"));
            
            function<void(void)>    fnBeforeAttempt = [this]() { StartGPS(); };
            function<void(void)>    fnAfterAttempt  = [this]() { StopGPS(); };
            
            uint8_t gpsTimeLockOk =
                gps_.GetNewTimeMeasurementSynchronousTwoMinuteMarkUnderWatchdog(
                    &gpsTimeMeasurement_,
                    cfg_.gpsMaxDurationTimeLockWaitMs,
                    fnBeforeAttempt,
                    fnAfterAttempt
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
            
            Log(gpsTimeLockOk ? P("  OK") : P("  Not OK"));
            
            
            /////////////////////////////////////////
            //
            // Send WSPR messages
            //
            /////////////////////////////////////////

            if (gpsTimeLockOk)
            {
                // If time locked, prepare to transmit.
                uint8_t wsprChannel = PreSendMessage();
                
                // Test message before sending (but send regardless)
                Log(P("Radio on, chan "), wsprChannel);
                
                // Figure out how long we've been operating since the mark
                uint32_t timeDiff = PAL.Millis() - timeAtMark;
                
                // Wait for the :01 second mark after even minute, accounting for 
                // time which has elapsed since the even minute
                const uint32_t ONE_SECOND = 1000UL;
                if (timeDiff < ONE_SECOND)
                {
                    PAL.Delay(ONE_SECOND - timeDiff);
                }
                
                // Send WSPR messages
                
                // First send the FCC-mandated message which reveals the
                // true callsign
                Log(P("Transmitting literal message"));
                SendMessageLiteral();

                // We want to now wait for the correct time to send the next
                // message.
                //
                // Duration of transmission of the first:
                // 162 symbols at 1.4648 baud = 110.6 s (110.592 = 1m50.592)
                //
                // We started the last at 1 second after a 2 min mark.
                // We want to do that again.  That's 2 min from the last start.
                // Since the prior transmission took 110.6 sec, we have
                // 120 - 110.6 = 9.4 seconds to wait.
                const uint32_t MS_WAIT = 9400;
                Log(P("Waiting for next 2-min mark"));
                PAL.Delay(MS_WAIT);

                // Now send the second encoded message
                Log(P("Transmitting encoded message"));
                SendMessageEncoded();
                
                Log(P("Radio off"));
                PostSendMessage();
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
            }
        }
        
        
        /////////////////////////////////////////
        //
        // Sleep for however long
        //
        /////////////////////////////////////////

        // Schedule next wakeup
        uint32_t wakeAndEvaluateDelayMs = CalculateWakeup(gpsHadError);
        tedWake_.RegisterForTimedEvent(wakeAndEvaluateDelayMs);
        
        Log(P("Sleeping "), wakeAndEvaluateDelayMs);
        Log();
        
        // Disable watchdog as the main set of code which can hang is complete
        PAL.WatchdogDisable();
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
         *   Apply standard configuration values
         * 
         */
        
        if (gpsHadError)
        {
            retVal = cfg_.gpsDurationWaitPostResetMs;
        }
        else
        {
            // Apply altitude parameters
            if (gpsLocationMeasurement_.altitudeFt <= userConfig_.geo.lowHighAltitudeFtThreshold)
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
    

private:

    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
};


#endif  // __APP_PICO_TRACKER_WSPR_2_H__




























