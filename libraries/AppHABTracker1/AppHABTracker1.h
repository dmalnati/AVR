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
    , amt_(cfg_.pinTxEnable)
    , seqNo_(0)
    {
        // Nothing to do
    }
    
    ~AppHABTracker1() {}
    
    void Run()
    {
        // Support serial output at all times
        ss_.begin(BAUD);
        ss_.println("Starting");
        
        // Set up application
        PAL.PinMode(cfg_.pinLedHeartbeat, OUTPUT);
        PAL.DigitalWrite(cfg_.pinLedHeartbeat, HIGH);
        tedHeartbeat_.SetCallback([this](){ PAL.DigitalToggle(cfg_.pinLedHeartbeat); });
        tedHeartbeat_.RegisterForTimedEventInterval(cfg_.heartbeatBlinkIntervalMs);
        
        PAL.PinMode(cfg_.pinLedGpsLock, OUTPUT);
        PAL.DigitalWrite(cfg_.pinLedGpsLock, LOW);
        
        PAL.PinMode(cfg_.pinLedTransmitting, OUTPUT);
        PAL.DigitalWrite(cfg_.pinLedTransmitting, LOW);
        
        
        // SD Logger
        sdLogger_.Init();
        logOp_  = sdLogger_.GetFileHandle("log.txt");
        logCsv_ = sdLogger_.GetFileHandle("log.csv");
        ss_.println("Logger Done");

        // GPS
        gps_.Init();
        ss_.println("GPS Done");
        
        // Compass
        compass_.Init();
        ss_.println("Compass Done");
        
        // Barometer
        barometer_.Init();
        ss_.println("Barometer Done");
        
        // Transmitter
        amt_.SetRadioWarmupDurationMs(cfg_.radioWarmupDurationMs);
        amt_.SetFlagStartDurationMs(cfg_.flagStartDurationMs);
        amt_.SetFlagEndDurationMs(cfg_.flagEndDurationMs);
        amt_.SetTransmitCount(cfg_.transmitCount);
        amt_.SetDelayMsBetweenTransmits(cfg_.delayMsBetweenTransmits);
        amt_.Init();
        ss_.println("Transmitter Done");
        
        // Set up timed callback to drive operational cycle
        ted_.SetCallback([this](){ OnTimeout(); });
        ted_.RegisterForTimedEventInterval(cfg_.aprsReportIntervalMs);
        
        // Handle events
        ss_.println("Evm Begin");
        evm_.MainLoop();
    }

private:

    void OnTimeout()
    {
        ss_.println();
        ss_.println("OnTimeout");
        
        {
            if (gps_.GetMeasurement(&gpsMeasurement_))
            {
                //ss_.println("GPS Measurement Success");
                ss_.print("msSinceLastFix: "); ss_.println(gpsMeasurement_.msSinceLastFix);
                
                #if 0
                ss_.println("---------------");
                
                ss_.print("date          : "); ss_.println(gpsMeasurement_.date);
                ss_.print("time          : "); ss_.println(gpsMeasurement_.time);
                ss_.print("latitude      : "); ss_.println(gpsMeasurement_.latitude);
                ss_.print("longitude     : "); ss_.println(gpsMeasurement_.longitude);
                ss_.print("altitude      : "); ss_.println(gpsMeasurement_.altitude);

                #endif
            }
            else
            {
                ss_.println("GPS Measurement Fail");
            }
        }
        
        {
            if (barometer_.GetMeasurement(&barometerMeasurement_))
            {
                //ss_.println("Barometer Measurement Success");
                #if 0
                ss_.println("---------------");
                
                ss_.print("tempF: ");
                ss_.println(barometerMeasurement_.tempF);
                //ss_.print("tempC: ");
                //ss_.println(barometerMeasurement_.tempC);
                ss_.print("pressureMilliBarAbsolute: ");
                ss_.println(barometerMeasurement_.pressureMilliBarAbsolute);
                /*
                ss_.print("pressureInchesMercuryAbsolute: ");
                ss_.println(barometerMeasurement_.pressureInchesMercuryAbsolute);
                ss_.print("pressureKiloPascalsAbsolute: ");
                ss_.println(barometerMeasurement_.pressureKiloPascalsAbsolute);
                ss_.print("pressureMilliBarSeaLevelAdjusted: ");
                ss_.println(barometerMeasurement_.pressureMilliBarSeaLevelAdjusted);
                ss_.print("pressureInchesMercurySeaLevelAdjusted: ");
                ss_.println(barometerMeasurement_.pressureInchesMercurySeaLevelAdjusted);
                ss_.print("pressureKiloPascalsSeaLevelAdjusted: ");
                ss_.println(barometerMeasurement_.pressureKiloPascalsSeaLevelAdjusted);
                ss_.print("altitudeFeetDerived: ");
                ss_.println(barometerMeasurement_.altitudeFeetDerived);
                ss_.print("altitudeMetersDerived: ");
                ss_.println(barometerMeasurement_.altitudeMetersDerived);
                */
                #endif
            }
            else
            {
                //ss_.println("Barometer Measurement Fail");
            }
        }
        
        {
            if (compass_.GetMeasurement(&compassMeasurement_))
            {
                //ss_.println("Compass Measurement Success");
                #if 0
                ss_.println("---------------");
                
                ss_.print("accelX: ");
                ss_.println(compassMeasurement_.accelX);
                ss_.print("accelY: ");
                ss_.println(compassMeasurement_.accelY);
                ss_.print("accelZ: ");
                ss_.println(compassMeasurement_.accelZ);
                ss_.print("magX: ");
                ss_.println(compassMeasurement_.magX);
                ss_.print("magY: ");
                ss_.println(compassMeasurement_.magY);
                ss_.print("magZ: ");
                ss_.println(compassMeasurement_.magZ);
                ss_.print("tempF: ");
                ss_.println(compassMeasurement_.tempF);
                #endif
            }
            else
            {
                // Not actually possible
            }
        }

        
        {
            logCsv_->Append(gpsMeasurement_.msSinceLastFix);
            logCsv_->Append(',');
            logCsv_->Append(gpsMeasurement_.date);
            logCsv_->Append(',');
            logCsv_->Append(gpsMeasurement_.time);
            logCsv_->Append(',');
            logCsv_->Append(gpsMeasurement_.latitude);
            logCsv_->Append(',');
            logCsv_->Append(gpsMeasurement_.longitude);
            logCsv_->Append(',');
            logCsv_->Append(gpsMeasurement_.altitude);
            logCsv_->Append('\n');
        }
        
        
        // Check status of GPS Lock
        if (gpsMeasurement_.msSinceLastFix <= cfg_.gpsLockGoodAgeLimitMs)
        {
            PAL.DigitalWrite(cfg_.pinLedGpsLock, HIGH);
        }
        else
        {
            PAL.DigitalWrite(cfg_.pinLedGpsLock, LOW);
        }
        
        OnTransmit();
    }
    
    
    void OnTransmit()
    {
        ss_.println("Transmitting");
        
        AX25UIMessage &msg = *amt_.GetAX25UIMessage();

        msg.SetDstAddress(cfg_.dstCallsign, cfg_.dstSsid);
        msg.SetSrcAddress(cfg_.srcCallsign, cfg_.srcSsid);
        msg.AddRepeaterAddress(cfg_.repeaterCallsign, cfg_.repeaterSsid);

        // Add APRS data
        uint8_t *bufInfo    = NULL;
        uint8_t  bufInfoLen = 0;

        APRSPositionReportMessageHABTracker1 aprm;

        if (msg.GetUnsafePtrInfo(&bufInfo, &bufInfoLen))
        {
            aprm.SetTargetBuf(bufInfo, bufInfoLen);
        
            aprm.SetTimeLocal(19, 14, 7);
            aprm.SetLatitude(40, 44, 13.87);
            aprm.SetSymbolTableID('/');
            aprm.SetLongitude(-74, 2, 2.32);
            aprm.SetSymbolCode('O');
            
            // extended
            aprm.SetCommentCourseAndSpeed(273, 777);
            aprm.SetCommentAltitude(444);

            // my extensions
            aprm.SetCommentBarometricPressureBinaryEncoded(10132);   // sea level
            aprm.SetCommentTemperatureBinaryEncoded(72); // first thermometer, inside(?)
            aprm.SetCommentMagneticsBinaryEncoded(-0.2051, 0.0527, 0.0742);    // on my desk
            aprm.SetCommentAccelerationBinaryEncoded(56.7017, 1042.7856, -946.2891);    // on my desk, modified y
            aprm.SetCommentTemperatureBinaryEncoded(74); // the other thermometer, outside(?)
            aprm.SetCommentVoltageBinaryEncoded(4.723);

            ++seqNo_;
            aprm.SetCommentSeqNoBinaryEncoded(seqNo_);

            msg.AssertInfoBytesUsed(aprm.GetBytesUsed());
        }

        gps_.DisableSerialInput();
        PAL.DigitalWrite(cfg_.pinLedTransmitting, HIGH);
        amt_.Transmit();
        PAL.DigitalWrite(cfg_.pinLedTransmitting, LOW);
        gps_.EnableSerialInput();
        
        ss_.println("Transmit Complete");
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

    void Features()
    {
        // LED indicator as to whether GPS lock achieved?
    }
    
    void OperationalPhases()
    {
        /*
         * Should you transmit before having GPS lock?  Yes, why wait, other sensors work.
         *
         * 
         *
         *
         */
    }
    
    void FailureHardening()
    {
        // Protect against any device failing completely at any time.
        // I2C is known to be problematic with infinite while loops.
        
        // Where are you at risk of starving out processing GPS serial stream?
    }
    
    
    
    
    
    
    
private:
    
    class OperationalLogger
    {
    public:
    
    private:
    };
    
    
    class GPSClock
    {
    public:
        void SyncToGPS(uint32_t timeGps)
        {
            timeGps_       = timeGps;
            timeAtLastFix_ = PAL.Millis();
        }
        
        uint32_t GetTime()
        {
            return timeGps_ + (PAL.Millis() - timeAtLastFix_);
        }
    
    private:
        uint32_t timeGps_;
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
    
    AX25UIMessageTransmitter<> amt_;
    uint16_t                   seqNo_;
    

    TimedEventHandlerDelegate tedHeartbeat_;

    TimedEventHandlerDelegate ted_;
    
    
    
};


#endif  // __APP_HAB_TRACKER_1_H__



