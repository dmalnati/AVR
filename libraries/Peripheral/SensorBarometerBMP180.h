#ifndef __SENSOR_BAROMETER_BMP180_H__
#define __SENSOR_BAROMETER_BMP180_H__


#include "PAL.h"

// This set of files has two compilation warnings (not errors)
// (warning: array subscript has type 'char' [-Wchar-subscripts])
// (warning: unused variable 'x' [-Wunused-variable])
// SparkFun libs
#include "SFE_BMP180.h"


class SensorBarometerBMP180
{
    // 5ms
    static const uint8_t PRESSURE_RESOLUTION_LOW = BMP180_COMMAND_PRESSURE0;
    
    // 8ms
    static const uint8_t PRESSURE_RESOLUTION_STANDARD = BMP180_COMMAND_PRESSURE1;
    
    // 14ms
    static const uint8_t PRESSURE_RESOLUTION_HIGH = BMP180_COMMAND_PRESSURE2;
    
    // 26ms
    static const uint8_t PRESSURE_RESOLUTION_ULTRA_HIGH = BMP180_COMMAND_PRESSURE3;
    
    constexpr static const double RATIO_MILLIBAR_TO_INCHES_MERCURY = 0.0295333727;
    constexpr static const double RATIO_MILLIBAR_TO_KILOPASCALS    = 0.1;
    constexpr static const double RATIO_METERS_TO_FEET             = 3.28084;
    
    
public:
    SensorBarometerBMP180() {}
    ~SensorBarometerBMP180() {}
    
    void Init()
    {
        sensor_.begin();
    }
    
    struct Measurement
    {
        Measurement()
        : tempF(0)
        , tempC(0)
        , pressureMilliBarAbsolute(0)
        , pressureInchesMercuryAbsolute(0)
        , pressureKiloPascalsAbsolute(0)
        , pressureMilliBarSeaLevelAdjusted(0)
        , pressureInchesMercurySeaLevelAdjusted(0)
        , pressureKiloPascalsSeaLevelAdjusted(0)
        , altitudeFeetDerived(0)
        , altitudeMetersDerived(0)
        {
            // Nothing to do
        }
        
        int16_t tempF;
        int16_t tempC;
        
        // pressure
        int16_t pressureMilliBarAbsolute;
        int16_t pressureInchesMercuryAbsolute;
        int16_t pressureKiloPascalsAbsolute;
        
        // calculated sea-level pressure
        int16_t pressureMilliBarSeaLevelAdjusted;
        int16_t pressureInchesMercurySeaLevelAdjusted;
        int16_t pressureKiloPascalsSeaLevelAdjusted;

        // calculated altitude
        int32_t altitudeFeetDerived;
        int32_t altitudeMetersDerived;
    };
    
    uint8_t GetMeasurement(Measurement *m)
    {
        uint8_t retVal  = 0;
        uint8_t msDelay = 0;
        
        msDelay = sensor_.startTemperature();
        if (msDelay)
        {
            PAL.Delay(msDelay);
            
            double tempC = 0;
            if (sensor_.getTemperature(tempC))
            {
                m->tempC = tempC;
                m->tempF = (tempC * (9.0 / 5.0)) + 32;
                
                msDelay = sensor_.startPressure(PRESSURE_RESOLUTION_STANDARD);
                if (msDelay)
                {
                    PAL.Delay(msDelay);
                    
                    double milliBarAbsolute = 0;
                    if (sensor_.getPressure(milliBarAbsolute, tempC))
                    {
                        retVal = 1;
                        
                        // Fill out measurement structure
                        m->pressureMilliBarAbsolute = milliBarAbsolute;
                        
                        m->pressureInchesMercuryAbsolute =
                            milliBarAbsolute *
                            RATIO_MILLIBAR_TO_INCHES_MERCURY;
                            
                        m->pressureKiloPascalsAbsolute = 
                            milliBarAbsolute *
                            RATIO_MILLIBAR_TO_KILOPASCALS;
                    }
                }
            }
        }
        
        return retVal;
    }
    
    uint8_t GetMeasurementAndCalculatedValues(Measurement *m,
                                              double       currentAltitudeMeters)
    {
        uint8_t retVal = GetMeasurement(m);
        
        if (retVal)
        {
            // do pressure compensation
            double pressureMilliBarSeaLevelAdjusted = 
                sensor_.sealevel(m->pressureMilliBarAbsolute,
                                 currentAltitudeMeters);
            
            m->pressureMilliBarSeaLevelAdjusted =
                pressureMilliBarSeaLevelAdjusted;
                                 
            m->pressureInchesMercurySeaLevelAdjusted =
                pressureMilliBarSeaLevelAdjusted *
                RATIO_MILLIBAR_TO_INCHES_MERCURY;
                
            m->pressureKiloPascalsSeaLevelAdjusted = 
                pressureMilliBarSeaLevelAdjusted *
                RATIO_MILLIBAR_TO_KILOPASCALS;
                                 
            // Get derived altitude
            double altitudeMetersDerived = 
                sensor_.altitude(m->pressureMilliBarAbsolute,
                                 pressureMilliBarSeaLevelAdjusted);
                                 
            m->altitudeMetersDerived = altitudeMetersDerived;
            m->altitudeFeetDerived =
                altitudeMetersDerived *
                RATIO_METERS_TO_FEET;
        }
        
        return retVal;
    }

private:
    
    // 75 bytes mem used for this
    SFE_BMP180 sensor_;
};





#endif  // __SENSOR_BAROMETER_BMP180_H__




