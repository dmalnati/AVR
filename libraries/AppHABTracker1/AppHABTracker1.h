#ifndef __APP_HAB_TRACKER_1_H__
#define __APP_HAB_TRACKER_1_H__


#include "Evm.h"
#include "SoftwareSerial.h"
#include "TimedEventHandler.h"
#include "PeripheralOpenLog.h"
#include "SensorGPSUblox.h"
#include "SensorEcompassLSM303C.h"
#include "SensorBarometerBMP180.h"
#include "AX25UIMessageTransmitter.h"
#include "APRSPositionReportMessageHABTracker1.h"


struct AppHABTracker1Config
{
    // Application configuration
    uint8_t pinSerialOutput;
    
    uint32_t aprsReportIntervalMs;
    uint32_t logIntervalMs;
    
    uint8_t  pinLedHeartbeat;
    uint32_t heartbeatBlinkIntervalMs;
    
    uint8_t  pinLedGpsLock;
    uint32_t gpsLockGoodAgeLimitMs;
    
    uint8_t pinLedTransmitting;
    
    // SD Logger
    uint8_t pinSerialTxSdLogger;
    
    // GPS
    uint8_t pinSerialRxGPS;
    uint8_t pinSerialTxGPS;
    
    // APRS
    const char *dstCallsign;
    uint8_t     dstSsid;
    const char *srcCallsign;
    uint8_t     srcSsid;
    const char *repeaterCallsign;
    uint8_t     repeaterSsid;
    
    // Transmitter
    uint8_t  pinTxEnable;
    
    uint32_t radioWarmupDurationMs;
    
    uint32_t flagStartDurationMs;
    uint32_t flagEndDurationMs;
    uint8_t  transmitCount;
    uint32_t delayMsBetweenTransmits;
};


class AppHABTracker1
{
    static const uint8_t SD_LOGGER_FILE_HANDLE_COUNT = 2;
    
    using PeripheralOpenLogType = PeripheralOpenLog<SD_LOGGER_FILE_HANDLE_COUNT>;
    
    static const uint16_t BAUD = 9600;
    
    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER =  0;

    
public:
    AppHABTracker1(AppHABTracker1Config &cfg)
    : cfg_(cfg)
    , ss_(PAL.GetArduinoPinFromPhysicalPin(-1),
          PAL.GetArduinoPinFromPhysicalPin(cfg_.pinSerialOutput))
    , sdLogger_(cfg_.pinSerialTxSdLogger)
    , logOp_(NULL)
    , logCsv_(NULL)
    , gps_(cfg_.pinSerialRxGPS, cfg_.pinSerialTxGPS)
    , vccMeasurement_(0.0)
    , amt_(cfg_.pinTxEnable)
    , seqNo_(0)
    {
        // Nothing to do
    }
    
    void Run()
    {
        // Support serial output at all times
        ss_.begin(BAUD);
        ss_.println("Starting");
        
        // Initialize
        InitApplication();
        InitPeripherals();
        
        // Handle events
        ss_.println("Evm");
        evm_.MainLoop();
    }
    
private:
    
    ////////////////////////////////////////////////////////////////////////
    //
    // Initialization
    //
    ////////////////////////////////////////////////////////////////////////
    
    void InitApplication()
    {
        // Set up Status monitoring
        PAL.PinMode(cfg_.pinLedHeartbeat, OUTPUT);
        PAL.DigitalWrite(cfg_.pinLedHeartbeat, HIGH);
        tedHeartbeat_.SetCallback([this](){ OnTimeoutHeartbeat(); });
        tedHeartbeat_.RegisterForTimedEventInterval(cfg_.heartbeatBlinkIntervalMs);
        
        PAL.PinMode(cfg_.pinLedGpsLock, OUTPUT);
        PAL.DigitalWrite(cfg_.pinLedGpsLock, LOW);
        
        PAL.PinMode(cfg_.pinLedTransmitting, OUTPUT);
        PAL.DigitalWrite(cfg_.pinLedTransmitting, LOW);
        //ss_.println("LEDs");
        
        // Set up timed events to drive application
        tedTransmit_.SetCallback([this](){ OnTimeoutTransmit(); });
        tedTransmit_.RegisterForTimedEventInterval(cfg_.aprsReportIntervalMs);
        
        tedLog_.SetCallback([this](){ OnTimeoutLog(); });
        tedLog_.RegisterForTimedEventInterval(cfg_.logIntervalMs);
        //ss_.println("Callbacks");
    }
    
    void InitPeripherals()
    {
        // SD Logger
        sdLogger_.Init();
        logOp_  = sdLogger_.GetFileHandle("log.txt");
        logCsv_ = sdLogger_.GetFileHandle("log.csv");
        //ss_.println("Logger Done");

        // GPS
        gps_.Init();
        //ss_.println("GPS");
        
        // Compass
        compass_.Init();
        //ss_.println("Compass");
        
        // Barometer
        barometer_.Init();
        //ss_.println("Barometer");
        
        // Transmitter
        amt_.SetRadioWarmupDurationMs(cfg_.radioWarmupDurationMs);
        amt_.SetFlagStartDurationMs(cfg_.flagStartDurationMs);
        amt_.SetFlagEndDurationMs(cfg_.flagEndDurationMs);
        amt_.SetTransmitCount(cfg_.transmitCount);
        amt_.SetDelayMsBetweenTransmits(cfg_.delayMsBetweenTransmits);
        amt_.Init();
        //ss_.println("Transmitter");
    }


    ////////////////////////////////////////////////////////////////////////
    //
    // Application Event Driving and Status Monitoring
    //
    ////////////////////////////////////////////////////////////////////////
    
    void OnTimeoutHeartbeat()
    {
        PAL.DigitalToggle(cfg_.pinLedHeartbeat);
    }
    
    void OnTimeoutLog()
    {
        TakeMeasurements();
        SetGPSLockStatus();
        LogData();
    }
    
    void OnTimeoutTransmit()
    {
        TakeMeasurements();
        SetGPSLockStatus();
        Transmit();
        LogData();
    }
    
    void SetGPSLockStatus()
    {
        // Check status of GPS Lock
        if (gpsMeasurement_.msSinceLastFix <= cfg_.gpsLockGoodAgeLimitMs)
        {
            PAL.DigitalWrite(cfg_.pinLedGpsLock, HIGH);
        }
        else
        {
            PAL.DigitalWrite(cfg_.pinLedGpsLock, LOW);
        }        
    }


    ////////////////////////////////////////////////////////////////////////
    //
    // Sensor Reading
    //
    ////////////////////////////////////////////////////////////////////////

    void TakeMeasurements()
    {
        gps_.GetMeasurement(&gpsMeasurement_);
        gpsClock_.SyncToGPS(gpsMeasurement_.hour,
                            gpsMeasurement_.minute,
                            gpsMeasurement_.second);
        
        barometer_.GetMeasurement(&barometerMeasurement_);
        
        compass_.GetMeasurement(&compassMeasurement_);
        
        vccMeasurement_ = PAL.ReadVccMillivolts() / 1000.0;
    }
    
    
    ////////////////////////////////////////////////////////////////////////
    //
    // Logging
    //
    ////////////////////////////////////////////////////////////////////////
    
    void LogData()
    {
        ss_.println("LS");

        // Make use of the transmitter buffer and formatting capabilities but
        // not for actually transmitting.
        //
        // Instead, format up the same message and serialize to the SD card.
        //
        // This keeps the code uniform and the decoding done later will be
        // identical.
        
        AX25UIMessage &msg = *amt_.GetAX25UIMessage();

        uint8_t *bufInfo    = NULL;
        uint8_t  bufInfoLen = 0;

        if (msg.GetUnsafePtrInfo(&bufInfo, &bufInfoLen))
        {
            APRSPositionReportMessageHABTracker1 aprm;
            
            aprm.SetTargetBuf(bufInfo, bufInfoLen);
    
            // We don't want to increment the sequence number here, because it's
            // clearly a new entry when we add one, and more interesting to be
            // able to see from the ground how many packets got lost at a given
            // time.
            uint8_t incrSeqNo = 0;
            FillOutPositionReport(aprm, incrSeqNo);
            
            uint8_t bytesUsed = aprm.GetBytesUsed();
            
            if (bytesUsed)
            {
                // Write to SD card
                sdLogger_.Append(bufInfo, bytesUsed);
                sdLogger_.Append('\n');
                
                // Debug
                ss_.write(bufInfo, bytesUsed);
                ss_.println();
            }
        }
        
        ss_.println("LE");
        ss_.println();
    }
    
    
    ////////////////////////////////////////////////////////////////////////
    //
    // Transmitting
    //
    ////////////////////////////////////////////////////////////////////////
    
    void Transmit()
    {
        ss_.println("TS");
        
        // Fill out basic AX.25 UI Frame details
        AX25UIMessage &msg = *amt_.GetAX25UIMessage();

        msg.SetDstAddress(cfg_.dstCallsign, cfg_.dstSsid);
        msg.SetSrcAddress(cfg_.srcCallsign, cfg_.srcSsid);
        msg.AddRepeaterAddress(cfg_.repeaterCallsign, cfg_.repeaterSsid);

        // Add APRS data to the payload
        uint8_t *bufInfo    = NULL;
        uint8_t  bufInfoLen = 0;

        if (msg.GetUnsafePtrInfo(&bufInfo, &bufInfoLen))
        {
            APRSPositionReportMessageHABTracker1 aprm;
            
            aprm.SetTargetBuf(bufInfo, bufInfoLen);
        
            uint8_t incrSeqNo = 1;
            FillOutPositionReport(aprm, incrSeqNo);
        
            msg.AssertInfoBytesUsed(aprm.GetBytesUsed());
        }

        // Transmit
        //
        // Turning off the GPS serial input avoids interrupts from disrupting
        // the operation of the modem.
        //
        // The LED indicating transmitting is operated here as well.
        //
        gps_.DisableSerialInput();
        PAL.DigitalWrite(cfg_.pinLedTransmitting, HIGH);
        
        amt_.Transmit();
        
        PAL.DigitalWrite(cfg_.pinLedTransmitting, LOW);
        gps_.EnableSerialInput();
        
        ss_.println("TE");
    }
    
    
    ////////////////////////////////////////////////////////////////////////
    //
    // APRS Message Formatting
    //
    ////////////////////////////////////////////////////////////////////////
    
    void FillOutPositionReport(APRSPositionReportMessageHABTracker1 &aprm,
                               uint8_t                              incrSeqNo)
    {
        // Set standard APRS Position Report fields
        //GPSClock::Time t = gpsClock_.GetTime();
        
        ss_.print("h:"); ss_.println(gpsMeasurement_.hour);
        ss_.print("m:"); ss_.println(gpsMeasurement_.minute);
        ss_.print("s:"); ss_.println(gpsMeasurement_.second);
        // ss_.print("h:"); ss_.println(t.hour);
        // ss_.print("m:"); ss_.println(t.minute);
        // ss_.print("s:"); ss_.println(t.second);
        
        aprm.SetTimeLocal(gpsMeasurement_.hour,
                          gpsMeasurement_.minute,
                          gpsMeasurement_.second);
        // aprm.SetTimeLocal(t.hour,
                          // t.minute,
                          // t.second);
        aprm.SetLatitude(gpsMeasurement_.latitudeDegrees,
                         gpsMeasurement_.latitudeMinutes,
                         gpsMeasurement_.latitudeSeconds);
        aprm.SetSymbolTableID('/');
        aprm.SetLongitude(gpsMeasurement_.longitudeDegrees,
                          gpsMeasurement_.longitudeMinutes,
                          gpsMeasurement_.longitudeSeconds);
        aprm.SetSymbolCode('O');
        
        // Set extended standard fields
        aprm.SetCommentCourseAndSpeed(gpsMeasurement_.courseDegrees,
                                      gpsMeasurement_.speedKnots);
        aprm.SetCommentAltitude(gpsMeasurement_.altitudeFt);

        
        // Set my own custom fields
        
        // BMP180
        aprm.SetCommentBarometricPressureBinaryEncoded(barometerMeasurement_.pressureMilliBarAbsolute);
        aprm.SetCommentTemperatureBinaryEncoded(barometerMeasurement_.tempF);
        
        // LSM303C
        aprm.SetCommentMagneticsBinaryEncoded(compassMeasurement_.magX,
                                              compassMeasurement_.magY,
                                              compassMeasurement_.magZ);
        aprm.SetCommentAccelerationBinaryEncoded(compassMeasurement_.accelX,
                                                 compassMeasurement_.accelY,
                                                 compassMeasurement_.accelZ);
        aprm.SetCommentTemperatureBinaryEncoded(compassMeasurement_.tempF);
        
        aprm.SetCommentVoltageBinaryEncoded(vccMeasurement_);

        if (incrSeqNo)
        {
            ++seqNo_;
        }
        aprm.SetCommentSeqNoBinaryEncoded(seqNo_);
    }
    
    
    

    void Notes()
    {
        // The barometer can give sea-level-compensated pressure readings, which
        // are standard, by additional processing using the altitude the
        // real pressure reading was taken at.
        
        // The GPS altitude should be used as that parameter.
        
        // On startup, must assume all devices are in some bad prior state and
        // re-init them.
        
        // Also need to log state changes and events.
    }

    
    
private:
    
    class OperationalLogger
    {
    public:
    
    private:
    };
    
    
    class GPSClock
    {
        static const uint32_t MS_PER_HOUR   = 60 * 60 * 1000L;
        static const uint32_t MS_PER_MINUTE =      60 * 1000L;
        static const uint32_t MS_PER_SECOND =           1000L;

    public:
        void SyncToGPS(uint8_t hour, uint8_t minute, uint8_t second)
        {
            if (hour != gpsHour_ && minute != gpsMinute_ && second != gpsSecond_)
            {
                gpsHour_   = hour;
                gpsMinute_ = minute;
                gpsSecond_ = second;

                timeAtLastFix_ = PAL.Millis();
            }
        }
        
        struct Time
        {
            uint8_t hour   = 0;
            uint8_t minute = 0;
            uint8_t second = 0;
        };
        
        Time GetTime()
        {
            // Calculate duration since last sync
            uint32_t timeNow = PAL.Millis();
            
            uint32_t timeDiffMs = timeNow - timeAtLastFix_;
            
            uint32_t timeDiffRemainingMs = timeDiffMs;
            
            // Calculate time that has passed            
            uint8_t hourOffset = timeDiffRemainingMs / MS_PER_HOUR;
            timeDiffRemainingMs -= (hourOffset * MS_PER_HOUR);
            
            uint8_t minuteOffset = timeDiffRemainingMs / MS_PER_MINUTE;
            timeDiffRemainingMs -= (minuteOffset * MS_PER_MINUTE);
            
            uint8_t secondOffset = timeDiffRemainingMs / MS_PER_SECOND;
            
            
            // Add time to last known sync'd clock
            Time t;
            t.hour   = gpsHour_;
            t.minute = gpsMinute_;
            t.second = gpsSecond_;
            
            t.second += secondOffset;
            if (t.second >= 60)
            {
                minuteOffset += 1;
                
                t.second -= 60;
            }
            
            t.minute += minuteOffset;
            if (t.minute >= 60)
            {
                hourOffset += 1;
                
                t.minute -= 60;
            }
            
            t.hour += hourOffset;
            if (t.hour >= 24)
            {
                t.hour -= 24;
            }
            
            return t;
        }
    
    private:
        uint8_t gpsHour_;
        uint8_t gpsMinute_;
        uint8_t gpsSecond_;
        
        uint32_t timeAtLastFix_;
    };

    
private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
    AppHABTracker1Config &cfg_;
    
    SoftwareSerial ss_;
    
    PeripheralOpenLogType        sdLogger_;
    PeripheralOpenLogFileHandle *logOp_;
    PeripheralOpenLogFileHandle *logCsv_;
    
    SensorGPSUblox               gps_;
    SensorGPSUblox::Measurement  gpsMeasurement_;
    
    SensorEcompassLSM303C              compass_;
    SensorEcompassLSM303C::Measurement compassMeasurement_;
    
    SensorBarometerBMP180               barometer_;
    SensorBarometerBMP180::Measurement  barometerMeasurement_;
    
    double vccMeasurement_;
    
    AX25UIMessageTransmitter<> amt_;
    uint16_t                   seqNo_;
    
    
    GPSClock gpsClock_;
    

    TimedEventHandlerDelegate tedHeartbeat_;
    TimedEventHandlerDelegate tedTransmit_;
    TimedEventHandlerDelegate tedLog_;
    
    
    
};


#endif  // __APP_HAB_TRACKER_1_H__



