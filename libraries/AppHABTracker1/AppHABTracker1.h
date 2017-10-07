#ifndef __APP_HAB_TRACKER_1_H__
#define __APP_HAB_TRACKER_1_H__


#include "Evm.h"
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
    uint32_t reportIntervalMs;
    
    // SD Logger
    uint8_t pinSerialTxSdLogger;
    
    // GPS
    uint8_t pinSerialRxGPS;
    uint8_t pinSerialTxGPS;
    
    // APRS
    char    *dstCallsign;
    uint8_t  dstSsid;
    char    *srcCallsign;
    uint8_t  srcSsid;
    char    *repeaterCallsign;
    uint8_t  repeaterSsid;
};


class AppHABTracker1
{
    static const uint8_t SD_LOGGER_FILE_HANDLE_COUNT = 2;
    
    using PeripheralOpenLogType = PeripheralOpenLog<SD_LOGGER_FILE_HANDLE_COUNT>;
    
    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;

    
public:
    AppHABTracker1(AppHABTracker1Config &cfg)
    : cfg_(cfg)
    , sdLogger_(cfg_.pinSerialTxSdLogger)
    , logOp_(NULL)
    , logCsv_(NULL)
    , gps_(cfg_.pinSerialRxGPS, cfg_.pinSerialTxGPS)
    , seqNo_(0)
    {
        // Nothing to do
    }
    
    ~AppHABTracker1() {}
    
    void Run()
    {
        // Support serial output at all times
        Serial.begin(9600);
        Serial.println("Starting");
        
        // SD Logger
        sdLogger_.Init();
        logOp_  = sdLogger_.GetFileHandle("log.txt");
        logCsv_ = sdLogger_.GetFileHandle("log.csv");
        
        // GPS
        gps_.Init();
        
        // Compass
        compass_.Init();
        
        // Barometer
        barometer_.Init();
        
        // Transmitter
        amt_.Init();
        amt_.SetFlagStartDurationMs(300);
        amt_.SetFlagEndDurationMs(10);
        amt_.SetTransmitCount(2);
        amt_.SetDelayMsBetweenTransmits(2000);
        
        // Set up timed callback to drive operational cycle
        ted_.SetCallback([this](){ OnTimeout(); });
        ted_.RegisterForTimedEventInterval(cfg_.reportIntervalMs);
        
        // Handle events
        evm_.MainLoop();
    }

private:

    void OnTimeout()
    {
        Serial.println("OnTimeout");
        
        {
            if (gps_.GetMeasurement(&gpsMeasurement_))
            {
                Serial.println("GPS Measurement Success");
                #if 0
                Serial.println("---------------");
                
                Serial.print("msSinceLastFix: "); Serial.println(gpsMeasurement_.msSinceLastFix);
                Serial.print("date          : "); Serial.println(gpsMeasurement_.date);
                Serial.print("time          : "); Serial.println(gpsMeasurement_.time);
                Serial.print("latitude      : "); Serial.println(gpsMeasurement_.latitude);
                Serial.print("longitude     : "); Serial.println(gpsMeasurement_.longitude);
                Serial.print("altitude      : "); Serial.println(gpsMeasurement_.altitude);

                #endif
            }
            else
            {
                Serial.println("GPS Measurement Fail");
            }
        }
        
        {
            if (barometer_.GetMeasurement(&barometerMeasurement_))
            {
                Serial.println("Barometer Measurement Success");
                #if 0
                Serial.println("---------------");
                
                Serial.print("tempF: ");
                Serial.println(barometerMeasurement_.tempF);
                //Serial.print("tempC: ");
                //Serial.println(barometerMeasurement_.tempC);
                Serial.print("pressureMilliBarAbsolute: ");
                Serial.println(barometerMeasurement_.pressureMilliBarAbsolute);
                /*
                Serial.print("pressureInchesMercuryAbsolute: ");
                Serial.println(barometerMeasurement_.pressureInchesMercuryAbsolute);
                Serial.print("pressureKiloPascalsAbsolute: ");
                Serial.println(barometerMeasurement_.pressureKiloPascalsAbsolute);
                Serial.print("pressureMilliBarSeaLevelAdjusted: ");
                Serial.println(barometerMeasurement_.pressureMilliBarSeaLevelAdjusted);
                Serial.print("pressureInchesMercurySeaLevelAdjusted: ");
                Serial.println(barometerMeasurement_.pressureInchesMercurySeaLevelAdjusted);
                Serial.print("pressureKiloPascalsSeaLevelAdjusted: ");
                Serial.println(barometerMeasurement_.pressureKiloPascalsSeaLevelAdjusted);
                Serial.print("altitudeFeetDerived: ");
                Serial.println(barometerMeasurement_.altitudeFeetDerived);
                Serial.print("altitudeMetersDerived: ");
                Serial.println(barometerMeasurement_.altitudeMetersDerived);
                */
                #endif
            }
            else
            {
                Serial.println("Barometer Measurement Fail");
            }
        }
        
        {
            if (compass_.GetMeasurement(&compassMeasurement_))
            {
                Serial.println("Compass Measurement Success");
                #if 0
                Serial.println("---------------");
                
                Serial.print("accelX: ");
                Serial.println(compassMeasurement_.accelX);
                Serial.print("accelY: ");
                Serial.println(compassMeasurement_.accelY);
                Serial.print("accelZ: ");
                Serial.println(compassMeasurement_.accelZ);
                Serial.print("magX: ");
                Serial.println(compassMeasurement_.magX);
                Serial.print("magY: ");
                Serial.println(compassMeasurement_.magY);
                Serial.print("magZ: ");
                Serial.println(compassMeasurement_.magZ);
                Serial.print("tempF: ");
                Serial.println(compassMeasurement_.tempF);
                #endif
            }
            else
            {
                // Not actually possible
            }
        }

        Serial.println();
        
        
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
        
        
        OnTransmit();
    }
    
    
    void OnTransmit()
    {
        #if 0
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

        amt_.Transmit();
        #endif
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
    
    TimedEventHandlerDelegate ted_;
    
    
    
};


#endif  // __APP_HAB_TRACKER_1_H__



