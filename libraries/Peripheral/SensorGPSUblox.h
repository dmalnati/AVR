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
    static const uint16_t GPS_WARMUP_TIME_MS    = 2000;
    static const uint8_t  MAX_UBX_MESSAGE_SIZE  = 44;
    
public:
    SensorGPSUblox(int8_t pinRx, int8_t pinTx)
    : ss_(PAL.GetArduinoPinFromPhysicalPin(pinRx),
          PAL.GetArduinoPinFromPhysicalPin(pinTx))
    {
        // nothing to do
    }
    
    void Init(uint16_t messageIntervalMs = 0)
    {
        // Give the GPS a moment to warm up and do stuff
        PAL.Delay(GPS_WARMUP_TIME_MS);
        
        // Start serial comms
        ss_.begin(BAUD);
        ss_.listen();
        
        // Configure GPS
        SetHighAltitudeMode();
        EnableOnlyGGAAndRMC();
        
        if (messageIntervalMs)
        {
            SetMessageInterval(messageIntervalMs);
        }
        
        // Set timers to come read serial data periodically
        timer_.SetCallback([this]() { this->OnTimeout(); });
        timer_.RegisterForTimedEventInterval(POLL_PERIOD_MS);
    }
    
    void DisableSerialInput()
    {
        ss_.stopListening();
    }
    
    void EnableSerialInput()
    {
        ss_.begin(BAUD);
        ss_.listen();
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
        uint8_t  hundredths;
        uint32_t fixAge;
        
        uint32_t courseDegrees;
        uint32_t speedKnots;
        
        int16_t  latitudeDegrees;
        uint8_t  latitudeMinutes;
        double   latitudeSeconds;
        
        int16_t  longitudeDegrees;
        uint8_t  longitudeMinutes;
        double   longitudeSeconds;
        
        uint32_t altitudeFt;
    };
    
    uint8_t GetMeasurement(Measurement *m)
    {
        constexpr static const double CM_TO_FT = 0.0328084;
        
        uint8_t retVal = 0;
        
        // Ask TinyGPS for decoded data
        int32_t latitude;
        int32_t longitude;
        tgps_.get_position(&latitude, &longitude, &m->msSinceLastFix);
        tgps_.get_datetime(&m->date, &m->time, &m->msSinceLastFix);
        tgps_.crack_datetime(&m->year,
                             &m->month,
                             &m->day,
                             &m->hour,
                             &m->minute,
                             &m->second,
                             &m->hundredths,
                             &m->fixAge);
 
        m->courseDegrees = tgps_.course() / 100;    // convert from 100ths of a degree
        m->speedKnots    = tgps_.speed() / 100;     // convert from 100ths of a knot
 
        ConvertTinyGPSLatLongToDegreesMinutesSeconds(latitude,
                                                     m->latitudeDegrees,
                                                     m->latitudeMinutes,
                                                     m->latitudeSeconds);
        
        ConvertTinyGPSLatLongToDegreesMinutesSeconds(longitude,
                                                     m->longitudeDegrees,
                                                     m->longitudeMinutes,
                                                     m->longitudeSeconds);

        m->altitudeFt = tgps_.altitude() * CM_TO_FT; // convert from cm to ft
        
        // Check for valid data
        if (m->msSinceLastFix != TinyGPS::GPS_INVALID_AGE)
        {
            retVal = 1;
        }
        
        return retVal;
    }


private:

    void OnTimeout()
    {
        // We only want to consume serial data, not extract measurements.
        while (ss_.available() > 0)
        {
            tgps_.encode(ss_.read());
        }
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


    SoftwareSerial ss_;
    TinyGPS        tgps_;
    
    UbxMessage<MAX_UBX_MESSAGE_SIZE> ubxMessage_;
    
    TimedEventHandlerDelegate timer_;
};


#endif  // __SENSOR_GPS_UBLOX_H__


