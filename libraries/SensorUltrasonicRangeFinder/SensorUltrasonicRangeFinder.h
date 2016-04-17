#ifndef __SENSOR_ULTRASONIC_RANGE_FINDER_H__
#define __SENSOR_ULTRASONIC_RANGE_FINDER_H__


#include "PAL.h"

#include "NewPing.h"


class SensorUltrasonicRangeFinder
{
    static const uint8_t SAMPLE_COUNT = 1;
    
public:
    SensorUltrasonicRangeFinder(uint8_t pinTrigger, uint8_t pinResponse)
    : sensor_(PAL.GetArduinoPinFromPhysicalPin(pinTrigger),
              PAL.GetArduinoPinFromPhysicalPin(pinResponse))
    {
        // Nothing to do
    }

    SensorUltrasonicRangeFinder(uint8_t pinTriggerAndResponse)
    : SensorUltrasonicRangeFinder(pinTriggerAndResponse, pinTriggerAndResponse)
    {
        // Nothing to do
    }
    
    
    struct Measurement
    {
        Measurement() : rttUs(0), distIn(0), distCm(0) { }
        
        uint32_t rttUs;
        uint8_t  distIn;
        uint8_t  distCm;
    };
    
    uint8_t GetMeasurement(Measurement *m)
    {
        uint8_t retVal = 0;
        
        uint32_t rttUs = sensor_.ping_median(SAMPLE_COUNT);
        
        // Check for failure
        if (rttUs != 0)
        {
            m->rttUs  = rttUs;
            m->distIn = NewPingConvert(rttUs, US_ROUNDTRIP_IN);
            m->distCm = NewPingConvert(rttUs, US_ROUNDTRIP_CM);
        }
        else
        {
            retVal = 0;
        }
        
        return retVal;
    }
    

private:

    NewPing sensor_;
};



#endif  // __SENSOR_ULTRASONIC_RANGE_FINDER_H__











