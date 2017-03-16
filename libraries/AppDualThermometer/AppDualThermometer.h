#ifndef __APP_DUAL_THERMOMETER_H__
#define __APP_DUAL_THERMOMETER_H__


#include "Evm.h"
#include "TimedEventHandler.h"
#include "SensorTemperatureDS18B20.h"
#include "ServoController.h"


struct AppDualThermometerConfig
{
    uint8_t pinThermometer1;
    uint8_t pinServo1;
    
    uint8_t pinThermometer2;
    uint8_t pinServo2;
};

class AppDualThermometer
{
private:

    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;

    //static const uint32_t POLL_MS_TEMPERATURE = 60000;  // 1 minute
    static const uint32_t POLL_MS_TEMPERATURE = 5000;
    
    static const uint32_t DURATION_MS_MOTOR_RUN = 1500;
    
    
public:

    AppDualThermometer(AppDualThermometerConfig &cfg)
    : cfg_(cfg)
    , sensorTemp1_(cfg_.pinThermometer1)
    , servo1_(cfg_.pinServo1)
    , sensorTemp2_(cfg_.pinThermometer2)
    , servo2_(cfg_.pinServo2)
    {
        // Nothing to do
    }
    
    ~AppDualThermometer()
    {
        // Nothing to do
    }

    void Run()
    {
        Serial.begin(9600);
        
        Serial.println("Run");
        
        // Initialize sensors
        sensorTemp1_.Init();
        sensorTemp2_.Init();
        
        // Set motor initial position
        servo1_.MoveTo(90);
        servo2_.MoveTo(90);
        
        // Set up motors to not be noisy
        servo1_.SetMaxDurationMotorEnabled(DURATION_MS_MOTOR_RUN);
        servo1_.SetModeIgnoreMoveToCurrentPosition();
        servo2_.SetMaxDurationMotorEnabled(DURATION_MS_MOTOR_RUN);
        servo2_.SetModeIgnoreMoveToCurrentPosition();
        
        // Set up and run (immediately) temperature taking timers
        auto cbFn = [this](){
            MeasureAndMove(sensorTemp1_, servo1_);
            MeasureAndMove(sensorTemp2_, servo2_);
            
            // Debug
            Serial.println();
        };
        tedPollTemperature_.SetCallback(cbFn);
        tedPollTemperature_.RegisterForTimedEventInterval(POLL_MS_TEMPERATURE);
        cbFn();

        // Go
        evm_.MainLoop();
    }


private:

    static void MeasureAndMove(SensorTemperatureDS18B20 &sensor,
                               ServoController          &servo)
    {
        sensor.GetMeasurementAsync([&](SensorTemperatureDS18B20::MeasurementAsync ma){
            if (ma.retVal)
            {
                int16_t tempF = ma.m.tempF;
                
                // constrain to range 0 - 100
                if (tempF < 0)        { tempF = 0;   }
                else if (tempF > 100) { tempF = 100; }
                
                // map onto 180 degree range
                uint8_t deg = (tempF * 180.0 / 100.0);
                
                // turn servo
                servo.MoveTo(deg);
                
                Serial.print("TempF: ");
                Serial.print(ma.m.tempF);
                Serial.print(", deg: ");
                Serial.print(deg);
                Serial.println();
            }
        });
    }

    void StopMotors()
    {
        servo1_.Stop();
        servo2_.Stop();
    }




    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppDualThermometerConfig &cfg_;
    
    SensorTemperatureDS18B20 sensorTemp1_;
    ServoController          servo1_;
    
    SensorTemperatureDS18B20 sensorTemp2_;
    ServoController          servo2_;
    
    TimedEventHandlerDelegate tedPollTemperature_;

};


#endif  // __APP_DUAL_THERMOMETER_H__