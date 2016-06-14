#ifndef __SENSOR_GPS_UBLOX_H__
#define __SENSOR_GPS_UBLOX_H__


#include "SoftwareSerial.h"

#include "PAL.h"
#include "TimedEventHandler.h"

#include "TinyGPS.h"


template <uint8_t MAX_MSG_SIZE>
class UbxMessage
{
    static const uint8_t LEN_HEADER     = 6;
    static const uint8_t LEN_TRAILER    = 2;
    static const uint8_t LEN_RESERVED   = LEN_HEADER + LEN_TRAILER;
    
    static const uint8_t BUF_SIZE = MAX_MSG_SIZE < LEN_RESERVED ?
                                    LEN_RESERVED                :
                                    MAX_MSG_SIZE;
                                    
    static const uint8_t CHECKSUM_START = 2;
    static const uint8_t LENGTH_START   = 4;
    
public:
    UbxMessage()
    {
        Reset();
    }
    
    ~UbxMessage() {}
    
    void Reset()
    {
        bufIdx_ = LEN_HEADER;
        memset(buf_, 0, BUF_SIZE);
        
        buf_[0] = 0xB5; // Sync char 1
        buf_[1] = 0x62; // Sync char 2
    }
    
    void GetBuf(uint8_t **buf, uint8_t *bufLen)
    {
        FinalizeBuffer();
        
        *buf    = buf_;
        *bufLen = bufIdx_ + LEN_TRAILER;
    }
    
    void SetClass(uint8_t ubxClass)
    {
        buf_[2] = ubxClass;
    }
    
    void SetId(uint8_t ubxId)
    {
        buf_[3] = ubxId;
    }
    
    void AddFieldU1(uint8_t val)
    {
        AddData((void *)&val, sizeof(val));
    }
    
    void AddFieldU2(uint16_t val)
    {
        uint16_t valLittleEndian = PAL.htons(PAL.htons(val));
        
        AddData((void *)&valLittleEndian, sizeof(valLittleEndian));
    }
    
    void AddFieldU4(uint32_t val)
    {
        uint32_t valLittleEndian = PAL.htonl(PAL.htonl(val));
        
        AddData((void *)&valLittleEndian, sizeof(valLittleEndian));
    }
    
    void AddFieldI1(int8_t val)
    {
        AddData((void *)&val, sizeof(val));
    }
    
    void AddFieldI4(int32_t val)
    {
        uint32_t valLittleEndian = PAL.htonl(PAL.htonl(val));
        
        AddData((void *)&valLittleEndian, sizeof(valLittleEndian));
    }
    
    void AddFieldX2(uint16_t val)
    {
        AddFieldU2(val);
    }
    
    
private:

    void FinalizeBuffer()
    {
        CalculateAndStoreLength();
        CalculateAndStoreChecksum();
    }
    
    void CalculateAndStoreLength()
    {
        uint16_t length             = bufIdx_ - LEN_HEADER;
        uint16_t lengthLittleEndian = PAL.htons(PAL.htons(length));
        
        memcpy((void *)&(buf_[LENGTH_START]),
               (void *)&lengthLittleEndian,
               sizeof(lengthLittleEndian));
    }
    
    void CalculateAndStoreChecksum()
    {
        uint8_t ckA = 0;
        uint8_t ckB = 0;
        
        for (uint8_t i = CHECKSUM_START; i < bufIdx_; ++i)
        {
            uint8_t b = buf_[i];
            
            ckA += b;
            ckB += ckA;
        }
        
        buf_[bufIdx_ + 0] = ckA;
        buf_[bufIdx_ + 1] = ckB;
    }

    void AddData(void *bufData, uint8_t bufDataLen)
    {
        // Make sure there is space available for checksum
        if ((bufIdx_ + bufDataLen) <= (BUF_SIZE - LEN_TRAILER))
        {
            memcpy((void *)(&(buf_[bufIdx_])), bufData, bufDataLen);
        }
    }

    uint8_t buf_[BUF_SIZE];
    uint8_t bufIdx_;
};


class SensorGPSUblox
{
    static const uint32_t BAUD                  = 9600;
    static const uint32_t POLL_PERIOD_MS        = 25;
    static const uint16_t GPS_MESSAGE_INTERVAL  = 5000;
    static const uint16_t GPS_WARMUP_TIME_MS    = 2000;
    static const uint8_t  MAX_UBX_MESSAGE_SIZE  = 44;
    
public:
    SensorGPSUblox(int8_t pinRx, int8_t pinTx)
    : ss_(PAL.GetArduinoPinFromPhysicalPin(pinRx),
          PAL.GetArduinoPinFromPhysicalPin(pinTx))
    {
        // nothing to do
    }
    
    void Init()
    {
        // Give the GPS a moment to warm up and do stuff
        PAL.Delay(GPS_WARMUP_TIME_MS);
        
        // Start serial comms
        ss_.begin(BAUD);
        ss_.listen();
        
        // Configure GPS
        SetHighAltitudeMode();
        EnableOnlyGGAAndRMC();
        SetMessageInterval(GPS_MESSAGE_INTERVAL);
        
        // Set timers to come read serial data periodically
        timer_.SetCallback([this]() { this->OnTimeout(); });
        timer_.RegisterForTimedEventInterval(POLL_PERIOD_MS);
    }
    
    void OnTimeout()
    {
        // We only want to consume serial data, not extract measurements.
        while (ss_.available() > 0)
        {
            tgps_.encode(ss_.read());
        }
    }
    
    struct Measurement
    {
        uint32_t msSinceLastFix;
        
        uint32_t date;
        uint32_t time;
        
        int32_t  latitude;
        int32_t  longitude;
        
        uint32_t altitude;
    };
    
    uint8_t GetMeasurement(Measurement *m)
    {
        uint8_t retVal = 0;
        
        // Ask TinyGPS for decoded data
        tgps_.get_position(&m->latitude, &m->longitude, &m->msSinceLastFix);
        tgps_.get_datetime(&m->date, &m->time, &m->msSinceLastFix);
        m->altitude = tgps_.altitude();
        
        // Check for valid data
        if (m->msSinceLastFix != TinyGPS::GPS_INVALID_AGE)
        {
            retVal = 1;
        }
        
        return retVal;
    }
    
    


private:

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
        
        uint8_t ubxClassUbxIdRateListLen = sizeof(ubxClassUbxIdRateList) / 3;
        
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



    SoftwareSerial ss_;
    TinyGPS        tgps_;
    
    UbxMessage<MAX_UBX_MESSAGE_SIZE> ubxMessage_;
    
    TimedEventHandlerDelegate timer_;
};


#endif  // __SENSOR_GPS_UBLOX_H__


