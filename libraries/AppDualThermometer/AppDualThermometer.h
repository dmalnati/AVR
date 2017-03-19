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
    int8_t  servo1PhysicalCalibrationOffset;
    
    uint8_t pinThermometer2;
    uint8_t pinServo2;
    int8_t  servo2PhysicalCalibrationOffset;
};

class AppDualThermometer
{
private:

    static const uint8_t C_IDLE  = 2 * SensorTemperatureDS18B20::C_IDLE +
                                   2 * ServoController::C_IDLE;
    static const uint8_t C_TIMED = 2 * SensorTemperatureDS18B20::C_TIMED +
                                   2 * ServoController::C_TIMED          +
                                   1;   // ted
    static const uint8_t C_INTER = 2 * SensorTemperatureDS18B20::C_INTER +
                                   2 * ServoController::C_INTER;

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
        
        // Set up motors to be reverse-operated due to mounting
        servo1_.SetModeInverted();
        servo2_.SetModeInverted();
        
        // Set up motors to not be noisy
        servo1_.SetMaxDurationMotorEnabled(DURATION_MS_MOTOR_RUN);
        servo1_.SetModeIgnoreMoveToCurrentPosition();
        servo2_.SetMaxDurationMotorEnabled(DURATION_MS_MOTOR_RUN);
        servo2_.SetModeIgnoreMoveToCurrentPosition();
        
        // Set up and run (immediately) temperature taking timers
        auto cbFn = [this](){
            MeasureAndMove("Sensor 1",
                           sensorTemp1_,
                           servo1_,
                           cfg_.servo1PhysicalCalibrationOffset);
            MeasureAndMove("Sensor 2",
                           sensorTemp2_,
                           servo2_,
                           cfg_.servo2PhysicalCalibrationOffset);
            
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

    static void MeasureAndMove(const char               *debugStr,
                               SensorTemperatureDS18B20 &sensor,
                               ServoController          &servo,
                               int8_t                   &servoPhysicalCalibrationOffset)
    {
        sensor.GetMeasurementAsync([&, debugStr](SensorTemperatureDS18B20::MeasurementAsync ma){
            if (ma.retVal)
            {
                int16_t tempF = ma.m.tempF;
                
                // constrain to range 0 - 100
                if (tempF < 0)        { tempF = 0;   }
                else if (tempF > 100) { tempF = 100; }
                
                // map onto 180 degree range
                uint8_t deg = (tempF * 180.0 / 100.0);
                
                // adjust angle to account for physical mounting imprecision
                deg += servoPhysicalCalibrationOffset;
                
                // turn servo
                servo.MoveTo(deg);
                
                Serial.print(debugStr);
                Serial.print(" - ");
                Serial.print("TempF: ");
                Serial.print(ma.m.tempF);
                Serial.print(", deg: ");
                Serial.print(deg);
                Serial.println();
            }
            else
            {
                Serial.println("Could not read temp");
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