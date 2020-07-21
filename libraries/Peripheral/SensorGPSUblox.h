#ifndef __SENSOR_GPS_UBLOX_H__
#define __SENSOR_GPS_UBLOX_H__


#include <math.h>

#include "ThinSoftwareSerial.h"

#include "PAL.h"
#include "TimedEventHandler.h"

#include "TinyGPS.h"


class SensorGPSUblox
{
public:
    static const uint8_t C_IDLE  = 0;
    static const uint8_t C_TIMED = 1;
    static const uint8_t C_INTER = 0;
    
    // Account for the minimum of 1 NMEA sentences which have to be consumed
    // in order to get a fresh synchronous time lock.
    // This is ~70 bytes at 9600 baud.
    // Each byte is a bit under 1ms with 8N1 encoding, so we'll leave
    // it calculated as 70ms.
    static const uint32_t MIN_DELAY_NEW_TIME_LOCK_MS = 70;
    
    // Structure size 14
    struct MeasurementLocationDMS
    {
        int16_t  latitudeDegrees;
        uint8_t  latitudeMinutes;
        double   latitudeSeconds;
        
        int16_t  longitudeDegrees;
        uint8_t  longitudeMinutes;
        double   longitudeSeconds;
    };

    // Structure size 70 bytes
    struct Measurement
    {
        uint32_t clockTimeAtMeasurement;
        
        uint32_t msSinceLastFix;
        
        uint32_t date;
        uint32_t time;
        
        int16_t  year;
        uint8_t  month;
        uint8_t  day;
        uint8_t  hour;
        uint8_t  minute;
        uint8_t  second;
        uint16_t millisecond;
        uint32_t fixAge;
        
        uint32_t courseDegrees;
        uint32_t speedKnots;
        
        int32_t latitudeDegreesMillionths;
        int32_t longitudeDegreesMillionths;
        
        MeasurementLocationDMS locationDms;
        
        static const uint8_t MAIDENHEAD_GRID_LEN = 6;
        char maidenheadGrid[MAIDENHEAD_GRID_LEN + 1] = { 0 };
        
        uint32_t altitudeFt;
    };
    
    enum class ResetType : uint16_t
    {
        HOT  = 0x0000,
        WARM = 0x0001,
        COLD = 0xFFFF,
    };
    
    enum class ResetMode : uint8_t
    {
        HW                  = 0x00,
        SW                  = 0x01,
        SW_GPS_ONLY         = 0x02,
        HW_AFTER_SD         = 0x04,
        CONTROLLED_GPS_DOWN = 0x08,
        CONTROLLED_GPS_UP   = 0x09,
    };

    
private:
    static const uint32_t BAUD                  = 9600;
    static const uint32_t POLL_PERIOD_MS        = 25;
    static const uint16_t GPS_WARMUP_TIME_MS    = 200;
    
    using FnGetAbstractMeasurement               = uint8_t (SensorGPSUblox::*)(Measurement *);
    using FnGetNewAbstractMeasurementSynchronous = uint8_t (SensorGPSUblox::*)(Measurement *, uint32_t, uint32_t *);
    
    
public:

    ///////////////////////////////////////////////////////////////////////////
    //
    // Ctor / Init / Debug
    //
    ///////////////////////////////////////////////////////////////////////////

    SensorGPSUblox(int8_t pinRx, int8_t pinTx)
    : pinRx_(pinRx)
    , pinTx_(pinTx)
    , ss_(pinRx_, pinTx_)
    {
        // nothing to do
    }
    
    // All configuration has to take place post-Init, as messages are relayed
    // to the GPS, and that isn't possible until after Init.
    void Init()
    {
        // Give the GPS a moment to warm up and do stuff
        PAL.Delay(GPS_WARMUP_TIME_MS);

        // Interface with serial
        EnableSerialInput();
    }
    
    // Debug
    ThinSoftwareSerial &DebugGetSS()
    {
        return ss_;
    }
    
    TinyGPS &DebugGetTinyGPS()
    {
        return tgps_;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Fine-grained control over active interface to GPS to account for
    // garbage knockoff GPS modules which try to power themselves this way.
    //
    ///////////////////////////////////////////////////////////////////////////

    void DisableSerialInput()
    {
        // Drastic requirement discovered through trial an error when working
        // with knockoff gps module.
        PAL.PinMode(pinRx_, OUTPUT);
        PAL.DigitalWrite(pinRx_, LOW);
        
        ss_.stopListening();
        
        timer_.DeRegisterForTimedEvent();
    }
    
    void EnableSerialInput()
    {
        // Drastic requirement discovered through trial an error when working
        // with knockoff gps module.
        PAL.PinMode(pinRx_, INPUT);
        
        ss_.begin(BAUD);
        ss_.listen();
        
        // Set timers to come read serial data periodically
        timer_.SetCallback([this]() { OnPoll(); });
        timer_.RegisterForTimedEventInterval(POLL_PERIOD_MS);
    }
    
    void DisableSerialOutput()
    {
        // Not really disabled, obviously, more like the pin put low since there
        // are some garbage GPS modules which will sink huge current through
        // serial-in.
        //
        // This is a desperation workaround to allow calling code to deal with
        // that themselves.
        PAL.DigitalWrite(pinTx_, LOW);
    }
    
    void EnableSerialOutput()
    {
        PAL.DigitalWrite(pinTx_, HIGH);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Configure behavior of and Command GPS module (post-Init)
    //
    ///////////////////////////////////////////////////////////////////////////

    void SetHighAltitudeMode()
    {
        uint8_t bufLen = 44;
        uint8_t buf[] = {
            0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0x01, 0x00,
            0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x55, 0xB4,
        };
        
        ss_.write(buf, bufLen);
        
        // Sync-wait for all bytes to be written.
        // At 9600 baud, each byte is just under 1ms, safe to delay by bufLen.
        PAL.Delay(bufLen);
    }

    void SaveConfiguration()
    {
        uint8_t bufLen = 21;
        uint8_t buf[] = {
            0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0x00, 0x00,
            0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x01, 0x1B, 0xA9,
        };

        ss_.write(buf, bufLen);
        
        // Sync-wait for all bytes to be written.
        // At 9600 baud, each byte is just under 1ms, safe to delay by bufLen.
        PAL.Delay(bufLen);
    }
    
    // COLD HW reset
    void ResetModule()
    {
        uint8_t bufLen = 12;
        uint8_t buf[] = {
            0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0xFF, 0xFF,
            0x00, 0x00, 0x0C, 0x5D,
        };

        ss_.write(buf, bufLen);
        
        // Sync-wait for all bytes to be written.
        // At 9600 baud, each byte is just under 1ms, safe to delay by bufLen.
        PAL.Delay(bufLen);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Runtime interface, getting/reseting locks on GPS data
    //
    // 2 things you can use the GPS for:
    // - locking onto time
    // - locking onto location
    // 
    // There are multiple similar interfaces to interact with each type.
    //
    // Note: Once Init() is called, data from GPS (when on) is polled continuously
    //       and used to maintain state about whether a lock has been acquired.
    // 
    // Interfaces for time/location below fall into two categories:
    // - Accessing already-acquired data which may reveal we have a lock
    // - Synchronously discarding any lock and re-acquiring a lock
    //
    //
    // Accessing already-acquired data which may reveal we have a lock
    // ---------------------------------------------------------------
    // GetTimeMeasurement / GetLocationMeasurement
    // 
    // 
    // Synchronously discarding any lock and re-acquiring a lock
    // ---------------------------------------------------------
    // GetNewTimeMeasurementSynchronous              / GetNewLocationMeasurementSynchronous
    // GetNewTimeMeasurementSynchronousUnderWatchdog / GetNewLocationMeasurementSynchronousUnderWatchdog
    // GetNewTimeMeasurementSynchronousTwoMinuteMarkUnderWatchdog / -
    //
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void ResetFix()
    {
        tgps_.ResetFix();
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Time-related Locks
    //
    ///////////////////////////////////////////////////////////////////////////
    
    // May not return true if the async polling hasn't acquired enough data yet
    uint8_t GetTimeMeasurement(Measurement *m)
    {
        uint8_t retVal = 0;
        
        // Check if both necessary sentences have been received
        if (tgps_.HasTimeLock())
        {
            retVal = 1;
            
            // Tell software to move the locked time into a place where it can
            // be extracted through normal means.
            tgps_.RevealTimeLock();
            
            // Only the time-related fields will be valid
            GetMeasurementInternal(m);
        }
        
        return retVal;
    }
    
    // This function ignores the fact that other async polling events may be
    // going on.  They do not impact this functionality.
    uint8_t GetNewTimeMeasurementSynchronous(Measurement   *m,
                                             uint32_t       timeoutMs,
                                             uint32_t      *timeUsedMs = NULL)
    {
        return GetNewAbstractMeasurementSynchronous(&SensorGPSUblox::GetTimeMeasurement, m, timeoutMs, timeUsedMs);
    }

    uint8_t GetNewTimeMeasurementSynchronousUnderWatchdog(Measurement *m, uint32_t timeoutMs, uint32_t *timeUsedMs = NULL)
    {
        return GetNewAbstractMeasurementSynchronousUnderWatchdog(&SensorGPSUblox::GetNewTimeMeasurementSynchronous, m, timeoutMs, timeUsedMs);
    }
    
    uint8_t GetNewTimeMeasurementSynchronousTwoMinuteMarkUnderWatchdog(
        Measurement             *m,
        uint32_t                 durationEachAttemptMs,
        function<void(void)>    &fnBeforeAttempt,
        function<void(void)>    &fnAfterAttempt)
    {
        uint8_t retVal = 0;
        
        fnBeforeAttempt();
        uint8_t gpsLockOk = GetNewTimeMeasurementSynchronousUnderWatchdog(m, durationEachAttemptMs);
        fnAfterAttempt();
        
        // Sleep until next GPS lock time or two min mark, whichever is
        // appropriate.
        uint32_t durationActualBeforeTwoMinMark = 0;
        if (gpsLockOk)
        {
            // Now we're able to know how long we are before the two min mark by
            // looking at the GPS data.
            // Instead of running the GPS constantly, sleep off enough time that
            // we can take one final shot at a fresh lock right beforehand and
            // fine tune our internal clock targeting by using a fresh GPS
            // time lock.
            uint32_t durationTargetBeforeTwoMinMarkToWakeMs = (durationEachAttemptMs + 2000);
            durationActualBeforeTwoMinMark                  = SleepToCloseToTwoMinMark(m, durationTargetBeforeTwoMinMarkToWakeMs);
        }
        
        // If time available to get another GPS lock, do it.
        if (gpsLockOk)
        {
            // We now know the actual duration remaining before the two min mark.
            // Possible we don't have time to try to lock again, due to,
            // for example, getting a time lock with 1 second remaining before
            // the two minute mark.  This is ok, we'll skip the second lock
            // in this case.
            if (durationActualBeforeTwoMinMark >= durationEachAttemptMs)
            {
                // We don't record the success of this lock, because whether it
                // did or didn't, we have a valid lock from previously that
                // we're going to use for the final sleep
                fnBeforeAttempt();
                GetNewTimeMeasurementSynchronousUnderWatchdog(m, durationEachAttemptMs);
                fnAfterAttempt();
            }
        }
        
        // Sleep until two min mark.  Either because we got a new more precise
        // time lock, or because we are burning off the remaining time from
        // the original lock.
        if (gpsLockOk)
        {
            // We have now locked once, and we're about to sleep off any
            // remaining time before the two min mark.
            // Whether we got two locks or one, this code should execute in
            // order to do our best to lock to the two min mark.  Therefore this
            // is a success.
            retVal = 1;
            
            // We want to now burn off remaining time (probably seconds)
            // right up to the two minute mark.
            // If we succeeded at the last lock attempt, we'll be using
            // that time as the reference.  Otherwise we're using the
            // original lock time as reference.
            SleepToCloseToTwoMinMark(m, 0);
        }
        
        return retVal;
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Location-related Locks
    //
    ///////////////////////////////////////////////////////////////////////////
    
    // May not return true if the async polling hasn't acquired enough data yet
    uint8_t GetLocationDMSMeasurement(MeasurementLocationDMS *mdms)
    {
        uint8_t retVal = 0;
        
        // Check if both necessary sentences have been received
        if (tgps_.GetLastTimeRMC() != 0 && tgps_.GetLastTimeGGA() != 0)
        {
            retVal = 1;
            
            GetMeasurementLocationDMSInternal(mdms);
        }
        
        return retVal;
    }

    // May not return true if the async polling hasn't acquired enough data yet
    uint8_t GetLocationMeasurement(Measurement *m)
    {
        uint8_t retVal = 0;
        
        // Check if both necessary sentences have been received
        if (tgps_.GetLastTimeRMC() != 0 && tgps_.GetLastTimeGGA() != 0)
        {
            retVal = 1;
            
            GetMeasurementInternal(m);
        }
        
        return retVal;
    }
    
    // This function ignores the fact that other async polling events may be
    // going on.  They do not impact this functionality.
    uint8_t GetNewLocationMeasurementSynchronous(Measurement   *m,
                                                 uint32_t       timeoutMs,
                                                 uint32_t      *timeUsedMs = NULL)
    {
        return GetNewAbstractMeasurementSynchronous(&SensorGPSUblox::GetLocationMeasurement, m, timeoutMs, timeUsedMs);
    }
    
    uint8_t GetNewLocationMeasurementSynchronousUnderWatchdog(Measurement *m, uint32_t timeoutMs, uint32_t *timeUsedMs = NULL)
    {
        return GetNewAbstractMeasurementSynchronousUnderWatchdog(&SensorGPSUblox::GetNewLocationMeasurementSynchronous, m, timeoutMs, timeUsedMs);
    }


private:

    void GetMeasurementLocationDMSInternal(MeasurementLocationDMS *mdms)
    {
        int32_t latitudeDegreesMillionths;
        int32_t longitudeDegreesMillionths;
        uint32_t msSinceLastFix;

        tgps_.get_position(latitudeDegreesMillionths,
                           longitudeDegreesMillionths,
                           msSinceLastFix);

        ConvertTinyGPSLatLongToDegreesMinutesSeconds(latitudeDegreesMillionths,
                                                     mdms->latitudeDegrees,
                                                     mdms->latitudeMinutes,
                                                     mdms->latitudeSeconds);
        
        ConvertTinyGPSLatLongToDegreesMinutesSeconds(longitudeDegreesMillionths,
                                                     mdms->longitudeDegrees,
                                                     mdms->longitudeMinutes,
                                                     mdms->longitudeSeconds);
    }
    
    void GetMeasurementInternal(Measurement *m)
    {
        constexpr static const double CM_TO_FT = 0.0328084;
        
        // temporary variables for some values we don't care to retain
        uint32_t fixAge     = 0;
        uint8_t  hundredths = 0;
        
        // Timestamp this moment as being the moment the lock was considered acquired
        m->clockTimeAtMeasurement = PAL.Millis();
        
        // Ask TinyGPS for decoded data
        tgps_.get_position(m->latitudeDegreesMillionths,
                           m->longitudeDegreesMillionths,
                           m->msSinceLastFix);
        tgps_.get_datetime(m->date, m->time, fixAge);
        tgps_.crack_datetime(m->year,
                             m->month,
                             m->day,
                             m->hour,
                             m->minute,
                             m->second,
                             hundredths,
                             m->fixAge);
        m->millisecond = hundredths * 10;
 
        m->courseDegrees = tgps_.course() / 100;    // convert from 100ths of a degree
        m->speedKnots    = tgps_.speed() / 100;     // convert from 100ths of a knot
 
        GetMeasurementLocationDMSInternal(&m->locationDms);

        ConvertToMaidenheadGrid(m->latitudeDegreesMillionths,
                                m->longitudeDegreesMillionths,
                                m->maidenheadGrid);
        
        int32_t altitude = tgps_.altitude();
        if (altitude < 0)
        {
            m->altitudeFt = 0;
        }
        else
        {
            m->altitudeFt = altitude * CM_TO_FT; // convert from cm to ft
        }
    }

    void OnPoll()
    {
        // We only want to consume serial data, not extract measurements.
        while (ss_.available() > 0)
        {
            uint8_t b = ss_.read();
            
            tgps_.encode(b);
        }
    }
    
    
public:

    /* 
     * Formats
     * -------
     * LAT:  4044.21069,N in TinyGPS is  40736878
     * LNG: 07402.03027,W in TinyGPS is -74033790
     *
     * The above GPGGA NMEA message lat/long values are defined as
     * being degrees and minutes with fractional minutes
     * eg: 4807.038,N   Latitude 48 deg 07.038' N
     * (http://www.gpsinformation.org/dale/nmea.htm#GGA)
     *
     * Whereas the TinyGPS values are in millionths of a degree.
     * Their example is:
     *   so instead of 90°30’00″, get_position() returns a longitude
     *        value of 90,500,000, or 90.5 degrees.
     * (http://arduiniana.org/libraries/tinygps/)
     *
     *
     * Convert from TinyGPS to deg/min/sec
     * -----------------------------------
     * 40736878
     * 40 and (0.736878 * 60) minutes
     * -> 40 deg 44.21268 minutes
     *
     * extract seconds from 44.21268 minutes
     * .21268 * 60 = 12.7608 seconds
     *
     * so should be
     * 40 deg 44 min 12.7608 sec
     *
     * or in Google Maps terms
     * 40 44 12.7608
     *
     *
     * Convert from GPS to deg/min/sec
     * -------------------------------
     * 4044.21069,N
     *
     * 40 deg and 44.21069 minutes
     *
     * extract seconds from 44.21069 minutes
     * .21069 * 60 = 12.6414 seconds
     *
     * so should be
     * 40 deg 44 min 12.6414 seconds
     *
     * or in Google Maps terms
     * 40 44 12.6414
     *
     *
     * Google Maps Notation
     * --------------------
     * Google Maps for TinyGPS Conversion
     * 40 44 12.7608, -74 2 2.32
     *
     * Google Maps for GPS Conversion
     * 40 44 12.6414, -74 2 2.32
     *
     * They're right next to each other.  Ok tolerance.
     * https://www.google.com/maps/dir/40+44+12.6414,+-74+2+2.32/40.736878,-74.0339778/@40.7368945,-74.0342609,19.67z/data=!4m7!4m6!1m3!2m2!1d-74.0339778!2d40.7368448!1m0!3e3
     *
     */
    static void ConvertTinyGPSLatLongToDegreesMinutesSeconds(int32_t   latOrLong,
                                                             int16_t  &degrees,
                                                             uint8_t  &minutes,
                                                             double   &seconds)
    {
        static const uint32_t ONE_MILLION     = 1000000;
        static const uint8_t  MIN_PER_DEGREE  = 60;
        static const uint8_t  SECONDS_PER_MIN = 60;
        
        // Capture input value for manipulation
        double valRemaining = latOrLong;                                // eg 40736878
        
        // Calculate degrees
        degrees = valRemaining / ONE_MILLION;                           // eg 40
        
        // Calculate minutes by converting millionths of a degree
        valRemaining = fabs(valRemaining) -
                       (abs(degrees) * ONE_MILLION);                   // eg   736878
        valRemaining = (valRemaining / ONE_MILLION) * MIN_PER_DEGREE;  // eg   44.21268
        
        minutes = valRemaining;                                         // eg 44
        
        // Calculate seconds by converting the fractional minutes
        valRemaining -= minutes;                                        // eg .21268
        valRemaining *= SECONDS_PER_MIN;                                // eg 12.7608
        
        seconds = valRemaining;                                         // eg 12.7608
    }
    
    //
    // https://en.wikipedia.org/wiki/Maidenhead_Locator_System
    //
    // Implementation adapted from TT7:
    // https://github.com/TomasTT7/TT7F-Float-Tracker/blob/master/Software/ARM_WSPR.h
    // https://github.com/TomasTT7/TT7F-Float-Tracker/blob/master/Software/ARM_WSPR.c
    //
    // test here: http://k7fry.com/grid/
    //
    //  40.738059  -74.036227 -> FN20XR
    // -26.30196   -66.709667 -> FG63PQ
    // -31.951585  115.824861 -> OF78VB
    //  10.813707  106.609537 -> OK30HT
    //
    // Will fill out 6-char maidenhead grid
    static void ConvertToMaidenheadGrid(int32_t  latitudeDegreesMillionths,
                                        int32_t  longitudeDegreesMillionths,
                                        char    *grid)
    {
        // Put lat/long in 10-thousandths
        int32_t lat = latitudeDegreesMillionths  / 100;
        int32_t lng = longitudeDegreesMillionths / 100;
        
        // Put into grid space, no negative degrees
        lat += (90  * 10000UL);
        lng += (180 * 10000UL);
        
        grid[0] = 'A' +   (lng / 200000);
        grid[1] = 'A' +   (lat / 100000);
        grid[2] = '0' +  ((lng % 200000) / 20000);
        grid[3] = '0' +  ((lat % 100000) / 10000);
        grid[4] = 'A' + (((lng % 200000) % 20000) / 834);
        grid[5] = 'A' + (((lat % 100000) % 10000) / 417);
    }
    
private:
public: // for testing

    // This function ignores the fact that other async polling events may be
    // going on.  They do not impact this functionality.
    uint8_t GetNewAbstractMeasurementSynchronous(FnGetAbstractMeasurement  fn,
                                                 Measurement              *m,
                                                 uint32_t                  timeoutMs,
                                                 uint32_t                 *timeUsedMs = NULL)
    {
        ResetFix();
        
        uint8_t retVal = 0;
        
        uint32_t timeStart = PAL.Millis();
        
        uint8_t cont = 1;
        while (cont)
        {
            OnPoll();
            
            uint8_t measurementOk = (*this.*fn)(m);

            uint32_t timeUsedMsInternal = PAL.Millis() - timeStart;
            
            if (timeUsedMs)
            {
                *timeUsedMs = timeUsedMsInternal;
            }
            
            if (measurementOk)
            {
                retVal = 1;
                
                cont = 0;
            }
            else
            {
                if (timeUsedMsInternal >= timeoutMs)
                {
                    cont = 0;
                }
            }
        }
        
        return retVal;
    }
    
    uint8_t GetNewAbstractMeasurementSynchronousUnderWatchdog(FnGetNewAbstractMeasurementSynchronous  fn,
                                                              Measurement                            *m,
                                                              uint32_t                                timeoutMs,
                                                              uint32_t                               *timeUsedMs)
    {
        const uint32_t ONE_SECOND_MS = 1000L;
        
        uint8_t retVal = 0;
        
        uint32_t timeStart = PAL.Millis();
        
        uint32_t durationRemainingMs = timeoutMs;
        
        uint8_t cont = 1;
        while (cont)
        {
            // Kick the watchdog
            PAL.WatchdogReset();
            
            // Calculate how long to wait for, one second or less each time
            uint32_t timeoutMsGps = durationRemainingMs > ONE_SECOND_MS ? ONE_SECOND_MS : durationRemainingMs;
            
            // Attempt lock
            retVal = (*this.*fn)(m, timeoutMsGps, timeUsedMs);
            
            if (retVal)
            {
                cont = 0;
            }
            else
            {
                durationRemainingMs -= timeoutMsGps;
                
                if (durationRemainingMs == 0)
                {
                    cont = 0;
                }
            }
        }
        
        if (timeUsedMs)
        {
            *timeUsedMs = PAL.Millis() - timeStart;
        }
        
        return retVal;
    }
    
    // Return the duration remaining before the two min mark.
    // Ideally this would match the durationTargetBeforeTwoMinMarkToWakeMs, but several
    // factors make it very possible that we're not able to achieve this.
    //
    // If the two min mark is now or in the past, return 0.
    uint32_t SleepToCloseToTwoMinMark(Measurement *m, uint32_t durationTargetBeforeTwoMinMarkToWakeMs)
    {
        uint32_t retVal = 0;
        
        // Give this a convenient name
        uint32_t timeGpsLock = m->clockTimeAtMeasurement;
        
        // Determine how long after the GPS lock the time will be at the
        // 2 min mark.
        uint32_t durationBeforeMarkMs = CalculateDurationMsToNextTwoMinuteMark(m);
        
        // Consider whether you have any time to burn by sleeping to get to the
        // desired time before the mark.
        if (durationBeforeMarkMs <= durationTargetBeforeTwoMinMarkToWakeMs)
        {
            // Already at or within, and that's not even considering the fact
            // that timeNow is later than the time the lock occurred.
            
            // Calculate time remaining before the mark as the return value.
            uint32_t timeNow = PAL.Millis();
            uint32_t timeDiff = timeNow - timeGpsLock;
            
            if (timeDiff <= durationBeforeMarkMs)
            {
                retVal = durationBeforeMarkMs - timeDiff;
            }
            else
            {
                retVal = 0;
            }
        }
        else
        {
            // At the time of the lock, we were earlier than the duration before the
            // mark that we were aiming for.
            
            // We know we can sleep for this long because we know the duration
            // remaining to the mark is greater than how much before the mark
            // we're supposed to wake.
            //
            // Notably we're doing this calculation based on the time the lock
            // was actually acquired, not the current time.
            uint32_t durationToSleepMs = durationBeforeMarkMs - durationTargetBeforeTwoMinMarkToWakeMs;
            
            // Now account for the actual time possibly having eaten up part or
            // all of the sleep duration.
            uint32_t timeNow = PAL.Millis();
            uint32_t timeDiff = timeNow - timeGpsLock;
            
            if (timeDiff < durationToSleepMs)
            {
                // Ok, difference still small enough that we should sleep.
                // Remove the time skew from lock to now though so we actually
                // wake at the correct time.
                durationToSleepMs -= timeDiff;
                
                PAL.DelayUnderWatchdog(durationToSleepMs);
                
                retVal = durationTargetBeforeTwoMinMarkToWakeMs;
            }
            else
            {
                // Oops, enough time has passed since the gps lock that we're
                // at or within the window before the mark we're supposed to
                // wake at.
                uint32_t timeDiffTargetVsLock = durationBeforeMarkMs - durationTargetBeforeTwoMinMarkToWakeMs;
                
                if (timeDiff - timeDiffTargetVsLock < durationTargetBeforeTwoMinMarkToWakeMs)
                {
                    retVal = durationTargetBeforeTwoMinMarkToWakeMs - (timeDiff - timeDiffTargetVsLock);
                }
                else
                {
                    retVal = 0;
                }
            }
        }
        
        return retVal;
    }

    uint32_t CalculateDurationMsToNextTwoMinuteMark(Measurement *m)
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
        uint8_t currentMinIsOdd = m->minute & 0x01;
        uint8_t maximumMinutesBeforeMark = currentMinIsOdd ? 1 : 2;
        
        uint8_t markMin = (m->minute + maximumMinutesBeforeMark) % 60;
        
        // Determine how far into the future the mark time is.
        if (((m->minute + 1) % 60) == markMin)
        {
            // We're at most 1 minute away.
            maximumMinutesBeforeMark = 1;
        }
        else // (((m->minute + 2) % 60) == markMin)
        {
            // We're at most 2 minutes away.
            maximumMinutesBeforeMark = 2;
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
            (m->second * 1000L) -
            (m->millisecond);
        
        return durationBeforeMarkMs;
    }
    
    
private:

    uint8_t pinRx_;
    uint8_t pinTx_;
    
    ThinSoftwareSerial ss_;
    
    TinyGPS        tgps_;
    
    TimedEventHandlerDelegate timer_;
};


#endif  // __SENSOR_GPS_UBLOX_H__


