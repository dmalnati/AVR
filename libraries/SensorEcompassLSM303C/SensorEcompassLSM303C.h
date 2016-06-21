#ifndef __ECOMPASS_LSM303C_H__
#define __ECOMPASS_LSM303C_H__


#include "SparkFunLSM303C.h"


class SensorEcompassLSM303C
{
public:
    SensorEcompassLSM303C() {}
    ~SensorEcompassLSM303C() {}
    
    void Init()
    {
        
    }
    
    struct Measurement
    {
        
    };
    
    uint8_t GetMeasurement(Measurement *m)
    {
        uint8_t retVal = 0;
        
        
        return retVal;
    }

private:


};


#endif  // __ECOMPASS_LSM303C_H__



