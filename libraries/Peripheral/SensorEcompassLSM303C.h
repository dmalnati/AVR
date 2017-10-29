#ifndef __ECOMPASS_LSM303C_H__
#define __ECOMPASS_LSM303C_H__


// This set of files has tons of compilation warnings (not errors)
// (warning: narrowing conversion of '+QNaN' from 'double' to 'int16_t {aka int}' inside { } [-Wnarrowing])
// (warning: overflow in implicit constant conversion [-Woverflow])
// Thanks SparkFun.
#include "SparkFunLSM303C.h"


class SensorEcompassLSM303C
{
public:
    SensorEcompassLSM303C() {}
    ~SensorEcompassLSM303C() {}
    
    void Init()
    {
        sensor_.begin();
    }
    
    struct Measurement
    {
        // Units of G
        float accelX;
        float accelY;
        float accelZ;
        
        // Units of Gauss
        float magX;
        float magY;
        float magZ;
        
        float tempF;
    };
    
    uint8_t GetMeasurement(Measurement *m)
    {
        uint8_t retVal = 1;
        
        // Re-begin on purpose.
        // If power is lost to the sensor after initial begin, all Accel values
        // become NaN.
        // This call makes them resolve again at a cost of approx +2.5ms to
        // a total of ~8ms.
        sensor_.begin();
        
        m->accelX = sensor_.readAccelX();
        m->accelY = sensor_.readAccelY();
        m->accelZ = sensor_.readAccelZ();
        
        m->magX   = sensor_.readMagX();
        m->magY   = sensor_.readMagY();
        m->magZ   = sensor_.readMagZ();
        
        m->tempF  = sensor_.readTempF();
        
        return retVal;
    }

private:

    LSM303C sensor_;
};


#endif  // __ECOMPASS_LSM303C_H__



