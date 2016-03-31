#ifndef __SENSOR_TEMPERATURE_DHTXX_H__
#define __SENSOR_TEMPERATURE_DHTXX_H__


#include "PAL.h"


// Only works for DHT11 at the moment.


class SensorTemperatureDHTXX
{
    static const uint32_t CALIBRATION_LOOP_COUNT       = 10000;
    static const uint32_t DURATION_US_BIT_ZERO_CEILING = 50;
    static const uint32_t DURATION_US_LONG_PULSE       = 8000;
    static const uint32_t DURATION_US_SHORT_PULSE      = 100;
    static const uint8_t  DEFAULT_ATTEMPT_COUNT        = 3;
    
public:
    SensorTemperatureDHTXX(uint8_t pin)
    : pin_(pin)
    {
        PAL.PinMode(pin_, INPUT_PULLUP);
    }
    
    ~SensorTemperatureDHTXX() { }
    
    void Init()
    {
        CalibrateTiming();
    }

    struct Measurement
    {
        uint8_t tempF;
        uint8_t tempC;
        uint8_t pctHumidity;
        uint8_t heatIndex;
    };
    
    uint8_t GetMeasurement(Measurement *m,
                           uint8_t      attemptCount = DEFAULT_ATTEMPT_COUNT)
    {
        uint8_t retVal = 0;
        
        if (m)
        {
            for (uint8_t i = 0; i < attemptCount && !retVal; ++i)
            {
                retVal = GetMeasurementInternal(m);
            }
        }
    
        return retVal;
    }
    

private:

    uint8_t GetMeasurementInternal(Measurement *m)
    {
        uint8_t byteArray[5] = { 0 };
        
        uint8_t cont = 1;

        
        // Suspend all interrupts for timing-sensitive areas.
        // Unfortunately this also screws up time keeping.
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Step 1 - Request data from sensor:
            // (from spec for DHT22)
            // - Default state of line should be HIGH
            // - Bring LOW for at least 500us or 18ms (which?)
            // - Bring HIGH for at least 18us
            
            PAL.PinMode(pin_, OUTPUT);
            
            PAL.DigitalWrite(pin_, LOW);
            PAL.Delay(1);
            
            // Restore pin to INPUT_PULLUP in order to:
            // - Allow sensor to now adjust the level
            // - Be able to read from the pin
            // - Set initial state for next time
            
            PAL.PinMode(pin_, INPUT_PULLUP);
            
            // Step 2 - Read response bit stream
            // Watch for 3 transitions to indicate incoming bit stream:
            // - HIGH to LOW, LOW to HIGH, HIGH to LOW
            cont = WaitForLevelChange(loopCountLongDelay_);
            for (uint8_t i = 0; i < 2 && cont; ++i)
            {
                cont = WaitForLevelChange(loopCountShortDelay_);
            }
            
            // Now read in bit stream
            for (uint8_t i = 0; i < 40 && cont; ++i)
            {
                // Skip low period
                cont = WaitForLevelChange(loopCountShortDelay_);
                if (cont)
                {
                    // Observe high period duration
                    uint8_t bitValue;
                    
                    cont = CalculateBitValueByPulseHighDuration(&bitValue);
                    if (cont)
                    {
                        uint8_t byteArrayIdx = i >> 3;
                        
                        // Shift everything in the accumulator byte left,
                        // which defaults the lowest bit to zero.
                        byteArray[byteArrayIdx] <<= 1;
                        
                        // Conditionally set the lowest bit to 1
                        if (bitValue)
                        {
                            byteArray[byteArrayIdx] |= 0x01;
                        }
                    }
                }
            }
        }
        
        // Timing-sensitive area finished
        
        // Step 3 - Process collected bit stream
        if (cont)
        {
            // Apply checksum.
            // Bytes 1-4 are data.
            // Byte  5 is the sum of bytes 1-4.
            cont = (byteArray[0] +
                    byteArray[1] +
                    byteArray[2] +
                    byteArray[3]) == byteArray[4];
            
            if (cont)
            {
                // Calculate Temperature
                m->tempC = byteArray[2];
                m->tempF = (uint8_t)(((float)m->tempC * (9.0 / 5.0)) + 32.0);
                
                // Calculate Humidity
                m->pctHumidity = byteArray[0];
                
                // Calculate Heat Index
                m->heatIndex = ComputeHeatIndex(m->tempF, m->pctHumidity);
            }
        }
        
        return cont;
    }

    // Lightly tweaked implementation taken from:
    // https://github.com/adafruit/DHT-sensor-library/blob/master/DHT.cpp
    //
    // See http://www.srh.noaa.gov/ama/?n=heatindex for a table of values.
    //
    //float DHT::computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit) {
    static uint8_t ComputeHeatIndex(float temperature, float percentHumidity)
    {
      // Using both Rothfusz and Steadman's equations
      // http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
      float hi;

      // if (!isFahrenheit)
        // temperature = convertCtoF(temperature);

      hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

      if (hi > 79) {
        hi = -42.379 +
                 2.04901523 * temperature +
                10.14333127 * percentHumidity +
                -0.22475541 * temperature*percentHumidity +
                -0.00683783 * pow(temperature, 2) +
                -0.05481717 * pow(percentHumidity, 2) +
                 0.00122874 * pow(temperature, 2) * percentHumidity +
                 0.00085282 * temperature*pow(percentHumidity, 2) +
                -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

        if((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
          hi -= ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

        else if((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
          hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
      }

      //return isFahrenheit ? hi : convertFtoC(hi);
      return (uint8_t)hi;
    }

    void CalibrateTiming()
    {
        // Measure duration of time taken to loop a known number of times.
        uint32_t timeStart = PAL.Micros();
        WaitForLevelChange(CALIBRATION_LOOP_COUNT);
        uint32_t timeEnd = PAL.Micros();
        
        uint32_t usPerLoopSet = timeEnd - timeStart;
        
        // Calculate delay loop counts
        loopCountLongDelay_ =
            (uint32_t)((double)DURATION_US_LONG_PULSE *
                       (double)CALIBRATION_LOOP_COUNT /
                       (double)usPerLoopSet);
        loopCountShortDelay_ =
            (uint32_t)((double)DURATION_US_SHORT_PULSE *
                       (double)CALIBRATION_LOOP_COUNT  /
                       (double)usPerLoopSet);
        loopCountBitZeroCeiling_ = 
            (uint32_t)((double)DURATION_US_BIT_ZERO_CEILING *
                       (double)CALIBRATION_LOOP_COUNT       /
                       (double)usPerLoopSet);
    }
    
    uint8_t WaitForLevelChange(uint32_t  loopCountLimit,
                               uint32_t *loopCountRet = NULL)
    {
        uint8_t level        = PAL.DigitalRead(pin_);
        uint8_t levelChanged = 0;
        
        uint8_t cont = 1;
        
        uint32_t loopCount = 0;
        
        while (cont)
        {
            if (level != PAL.DigitalRead(pin_))
            {
                cont = 0;
                
                levelChanged = 1;
            }
            else
            {
                ++loopCount;
                
                if (loopCount >= loopCountLimit)
                {
                    cont = 0;
                }
            }
        }
        
        if (loopCountRet)
        {
            *loopCountRet = loopCount;
        }
        
        return levelChanged;
    }
    
    uint8_t CalculateBitValueByPulseHighDuration(uint8_t *bitValue)
    {
        uint8_t retVal = 0;
        
        uint32_t loopCount = 0;
        
        if (WaitForLevelChange(loopCountShortDelay_, &loopCount))
        {
            retVal = 1;
            
            if (loopCount <= loopCountBitZeroCeiling_)
            {
                *bitValue = 0;
            }
            else
            {
                *bitValue = 1;
            }
        }

        return retVal;
    }


    Pin pin_;
    
    uint32_t loopCountLongDelay_;
    uint32_t loopCountShortDelay_;
    uint32_t loopCountBitZeroCeiling_;
};



#endif  // __SENSOR_TEMPERATURE_DHTXX_H__

