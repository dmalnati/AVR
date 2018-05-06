#ifndef __SENSOR_GPS_UBLOX_H__
#define __SENSOR_GPS_UBLOX_H__


#include <math.h>

#include "SoftwareSerial.h"

#include "PAL.h"
#include "TimedEventHandler.h"

#include "UbxMessage.h"

#include "TinyGPS.h"


class SensorGPSUblox
{
public:
    static const uint8_t C_IDLE  = 0;
    static const uint8_t C_TIMED = 1;
    static const uint8_t C_INTER = 0;
    
private:
    static const uint32_t BAUD                  = 9600;
    static const uint32_t POLL_PERIOD_MS        = 25;
    static const uint16_t GPS_WARMUP_TIME_MS    = 200;
    static const uint8_t  MAX_UBX_MESSAGE_SIZE  = 44;
    
public:
    SensorGPSUblox(int8_t pinRx, int8_t pinTx)
    : pinTx_(pinTx)
    , ss_(PAL.GetArduinoPinFromPhysicalPin(pinRx),
          PAL.GetArduinoPinFromPhysicalPin(pinTx_))
    {
        // nothing to do
    }
    
    // Debug
    SoftwareSerial &DebugGetSS()
    {
        return ss_;
    }
    
    TinyGPS &DebugGetTinyGPS()
    {
        return tgps_;
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
    
    void DisableSerialInput()
    {
        ss_.stopListening();
        
        timer_.DeRegisterForTimedEvent();
    }
    
    void EnableSerialInput()
    {
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
    
    void ResetFix()
    {
        tgps_.ResetFix();
    }
    
    void SetHighAltitudeMode()
    {
        ubxMessage_.Reset();
        
        // Create CFG-NAV5 msg
        // Set altitude mode to Airborne < 1g
        // Indicate this is the only parameter changing via bitmap
        ubxMessage_.SetClass(0x06);
        ubxMessage_.SetId(0x24);
        
        ubxMessage_.AddFieldX2(0x0001); // mask     = dynModel change only
        ubxMessage_.AddFieldU1(6);      // dynModel = airborne < 1g
        ubxMessage_.AddFieldU1(0);      // fixMode
        ubxMessage_.AddFieldI4(0);      // fixedAlt
        ubxMessage_.AddFieldU4(0);      // fixedAltVar
        ubxMessage_.AddFieldI1(0);      // minElev
        ubxMessage_.AddFieldU1(0);      // drLimit
        ubxMessage_.AddFieldU2(0);      // pDop
        ubxMessage_.AddFieldU2(0);      // tDop
        ubxMessage_.AddFieldU2(0);      // pAcc
        ubxMessage_.AddFieldU2(0);      // tAcc
        ubxMessage_.AddFieldU1(0);      // staticHoldThresh
        ubxMessage_.AddFieldU1(0);      // dgpsTimeOut
        ubxMessage_.AddFieldU4(0);      // reserved2
        ubxMessage_.AddFieldU4(0);      // reserved3
        ubxMessage_.AddFieldU4(0);      // reserved4
        
        uint8_t *buf;
        uint8_t  bufLen;
        ubxMessage_.GetBuf(&buf, &bufLen);
        
        ss_.write(buf, bufLen);
    }

    void SetMessageInterval(uint16_t intervalMs)
    {
        ubxMessage_.Reset();
        
        // Create CFG-RATE msg
        ubxMessage_.SetClass(0x06);
        ubxMessage_.SetId(0x08);
        
        ubxMessage_.AddFieldU2(intervalMs); // measRate
        ubxMessage_.AddFieldU2(1);          // navRate
        ubxMessage_.AddFieldU2(1);          // timeRef
        
        uint8_t *buf;
        uint8_t  bufLen;
        ubxMessage_.GetBuf(&buf, &bufLen);
        
        ss_.write(buf, bufLen);
    }
    
    void EnableOnlyGGAAndRMC()
    {
        // These are the messages broadcast by default.
        // Specifically enable and disable to be certain.
        uint8_t ubxClassUbxIdRateList[] = {
            0xF0, 0x00, 1,  // GGA
            0xF0, 0x01, 0,  // GLL
            0xF0, 0x02, 0,  // GSA
            0xF0, 0x03, 0,  // GSV
            0xF0, 0x04, 1,  // RMC
            0xF0, 0x05, 0   // VTG
        };
        
        uint8_t ubxClassUbxIdRateListLen = sizeof(ubxClassUbxIdRateList);
        
        for (uint8_t i = 0; i < ubxClassUbxIdRateListLen; i += 3)
        {
            uint8_t ubxClass = ubxClassUbxIdRateList[i + 0];
            uint8_t ubxId    = ubxClassUbxIdRateList[i + 1];
            uint8_t rate     = ubxClassUbxIdRateList[i + 2];
            
            SetMessageRate(ubxClass, ubxId, rate);
        }
    }
    
    void SaveConfiguration()
    {
        ubxMessage_.Reset();

        // CFG-CFG (0x06 0x09)
        ubxMessage_.SetClass(0x06);
        ubxMessage_.SetId(0x09);

        ubxMessage_.AddFieldX4(0);           // clearMask  - clear nothing
        ubxMessage_.AddFieldX4(0x0000FFFF);  // saveMask   - save everything
        ubxMessage_.AddFieldX4(0);           // loadMask   - load nothing
        ubxMessage_.AddFieldX1(1);           // deviceMask - save to batter-backed ram (internal to chip)

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMessage_.GetBuf(&buf, &bufLen);

        ss_.write(buf, bufLen);
    }
    
    struct Measurement
    {
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
        
        int16_t  latitudeDegrees;
        uint8_t  latitudeMinutes;
        double   latitudeSeconds;
        
        int16_t  longitudeDegrees;
        uint8_t  longitudeMinutes;
        double   longitudeSeconds;
        
        uint32_t altitudeFt;
    };
    
    // May not return true if the async polling hasn't acquired enough data yet
    uint8_t GetMeasurement(Measurement *m)
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
    uint8_t GetNewMeasurementSynchronous(Measurement   *m,
                                         uint32_t       timeoutMs,
                                         uint32_t      *timeUsedMs = NULL)
    {
        ResetFix();
        
        uint8_t retVal = 0;
        
        uint32_t timeStart = PAL.Millis();
        
        uint8_t cont = 1;
        while (cont)
        {
            OnPoll();
            
            uint8_t measurementOk = GetMeasurement(m);

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
    
    uint8_t GetGPSLockUnderWatchdog(Measurement *m, uint32_t timeoutMs)
    {
        const uint32_t ONE_SECOND_MS = 1000L;
        
        uint8_t retVal = 0;
        
        uint32_t durationRemainingMs = timeoutMs;
        
        uint8_t cont = 1;
        while (cont)
        {
            // Kick the watchdog
            PAL.WatchdogReset();
            
            // Calculate how long to wait for, one second or less each time
            uint32_t timeoutMsGps = durationRemainingMs > ONE_SECOND_MS ? ONE_SECOND_MS : durationRemainingMs;
            
            // Attempt lock
            retVal = GetNewMeasurementSynchronous(m, timeoutMsGps);
            
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
        
        return retVal;
    }
    
    uint8_t WaitForNextGPSTwoMinuteMark(Measurement *m)
    {
        const uint32_t TWO_MINUTES_MS = 2 * 60 * 1000L;
        
        // Initially we want to get a fresh lock since there is no guaranteed
        // prior state.  We don't care about the 2 minute mark yet.  We just
        // want the known current state.
        uint8_t gpsLockOk = GetGPSLockUnderWatchdog(m, TWO_MINUTES_MS);
        
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
            uint8_t currentMinIsOdd = m->minute & 0x01;
            uint8_t maximumMinutesBeforeMark = currentMinIsOdd ? 1 : 2;
            
            uint8_t markMin = (m->minute + maximumMinutesBeforeMark) % 60;

            
            // Track using GPS, or determine that we're already there
            uint8_t cont = 1;
            while (cont)
            {
                // Determine how far into the future the mark time is.
                if (((m->minute + 1) % 60) == markMin)
                {
                    // We're at most 1 minute away.
                    maximumMinutesBeforeMark = 1;
                }
                else if (((m->minute + 2) % 60) == markMin)
                {
                    // We're at most 2 minutes away.
                    maximumMinutesBeforeMark = 2;
                }
                else
                {
                    // Shouldn't be possible...
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
                    (m->second * 1000L) -
                    (m->millisecond);            
                
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
                    gpsLockOk = GetGPSLockUnderWatchdog(m, durationBeforeMarkMs + SYSTEM_CLOCK_FAST_PROTECTION_MS);
                    
                    if (gpsLockOk)
                    {
                        // Check if we're at the right time.
                        // If not, we'll calculate how long wait on the next
                        // loop iteration.
                        if (m->minute == markMin)
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


private:
    
    void GetMeasurementInternal(Measurement *m)
    {
        constexpr static const double CM_TO_FT = 0.0328084;
        
        // Ask TinyGPS for decoded data
        tgps_.get_position(&m->latitudeDegreesMillionths,
                           &m->longitudeDegreesMillionths,
                           &m->msSinceLastFix);
        tgps_.get_datetime(&m->date, &m->time, NULL);
        uint8_t hundredths;
        tgps_.crack_datetime(&m->year,
                             &m->month,
                             &m->day,
                             &m->hour,
                             &m->minute,
                             &m->second,
                             &hundredths,
                             &m->fixAge);
        m->millisecond = hundredths * 10;
 
        m->courseDegrees = tgps_.course() / 100;    // convert from 100ths of a degree
        m->speedKnots    = tgps_.speed() / 100;     // convert from 100ths of a knot
 
        ConvertTinyGPSLatLongToDegreesMinutesSeconds(m->latitudeDegreesMillionths,
                                                     m->latitudeDegrees,
                                                     m->latitudeMinutes,
                                                     m->latitudeSeconds);
        
        ConvertTinyGPSLatLongToDegreesMinutesSeconds(m->longitudeDegreesMillionths,
                                                     m->longitudeDegrees,
                                                     m->longitudeMinutes,
                                                     m->longitudeSeconds);

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
    
    
    void SetMessageRate(uint8_t ubxClass, uint8_t ubxId, uint8_t rate)
    {
        ubxMessage_.Reset();
        
        // Create CFG-MSG msg
        ubxMessage_.SetClass(0x06);
        ubxMessage_.SetId(0x01);
        
        ubxMessage_.AddFieldU1(ubxClass);   // msgClass
        ubxMessage_.AddFieldU1(ubxId);      // msgID
        ubxMessage_.AddFieldU1(rate);       // rate
        
        uint8_t *buf;
        uint8_t  bufLen;
        ubxMessage_.GetBuf(&buf, &bufLen);
        
        ss_.write(buf, bufLen);
    }
    
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
    void ConvertTinyGPSLatLongToDegreesMinutesSeconds(int32_t   latOrLong,
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

    uint8_t pinTx_;
    
    SoftwareSerial ss_;
    TinyGPS        tgps_;
    
    UbxMessage<MAX_UBX_MESSAGE_SIZE> ubxMessage_;
    
    TimedEventHandlerDelegate timer_;
};


#endif  // __SENSOR_GPS_UBLOX_H__


