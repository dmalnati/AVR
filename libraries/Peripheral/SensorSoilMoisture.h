#ifndef __SENSOR_SOIL_MOISTURE_H__
#define __SENSOR_SOIL_MOISTURE_H__


#include <stdint.h>

#include "PAL.h"


class SensorSoilMoisture
{
public:
    SensorSoilMoisture(uint8_t pinSensorPower, uint8_t pinSensorMeasurement)
    : pinSensorPower_(pinSensorPower)
    , pinSensorMeasurement_(pinSensorMeasurement)
    {
        PAL.PinMode(pinSensorPower_, OUTPUT);
        PAL.DigitalWrite(pinSensorPower_, LOW);
        
        PAL.PinMode(pinSensorMeasurement_, INPUT);
    }
    
    ~SensorSoilMoisture()
    {
        // Nothing to do
    }
    
    struct Measurement
    {
        uint8_t pctConductive;
    };
    
    uint8_t GetMeasurement(Measurement *m)
    {
        uint8_t retVal = 0;
        
        if (m)
        {
            retVal = 1;
            
            // Enable power
            PAL.DigitalWrite(pinSensorPower_, HIGH);
            
            // Wait(?)
            PAL.Delay(50);
            
            // Read analog value
            uint16_t adcVal = PAL.AnalogRead(pinSensorMeasurement_);
            
            // Disable power
            PAL.DigitalWrite(pinSensorPower_, LOW);
            
            
            static const uint16_t MAX_ADC_VALUE = 1024;
            
            
            LogNNL("adcVal: ");
            LOg(adcVal);
            LogNNL("MAX_ADC_VALUE: ");
            Log(MAX_ADC_VALUE);
            double pctConductiveFloat = ((double)adcVal / MAX_ADC_VALUE * 100.0);
            LogNNL("pctConductiveFloat: ");
            Log(pctConductiveFloat);
            
            // Convert to percentage
            m->pctConductive = (uint8_t)((double)adcVal / MAX_ADC_VALUE * 100.0);
        }
        
        return retVal;
    }

private:

    uint8_t pinSensorPower_;
    uint8_t pinSensorMeasurement_;
};


#endif  // __SENSOR_SOIL_MOISTURE_H__