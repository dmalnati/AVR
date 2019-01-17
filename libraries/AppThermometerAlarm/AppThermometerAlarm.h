#ifndef __APP_THERMOMETER_ALARM_H__
#define __APP_THERMOMETER_ALARM_H__


#include "Log.h"
#include "Evm.h"
#include "TimedEventHandler.h"
#include "SensorTemperatureDS18B20.h"
#include "LCDFrentaly20x4.h"



struct AppThermometerAlarmConfig
{
    uint8_t pinThermometer;
    uint8_t alarmTempF;
    uint8_t pinAlarm;
};


class AppThermometerAlarm
{
private:

    static const uint8_t C_IDLE  = SensorTemperatureDS18B20::C_IDLE;
    static const uint8_t C_TIMED = SensorTemperatureDS18B20::C_TIMED;
    static const uint8_t C_INTER = SensorTemperatureDS18B20::C_INTER;

    static const uint32_t POLL_MS_TEMPERATURE = 1000;
    
    
public:

    AppThermometerAlarm(AppThermometerAlarmConfig &cfg)
    : cfg_(cfg)
    , sensorTemp_(cfg_.pinThermometer)
    {
        // Nothing to do
    }
    
    ~AppThermometerAlarm()
    {
        // Nothing to do
    }

    void Run()
    {
        LogStart(9600);
        Log("Starting");

        // Init sensor
        sensorTemp_.Init();
        
        // Init LCD
        InitLCD();
        
        // Init alarm
        PAL.PinMode(cfg_.pinAlarm, OUTPUT);
        PAL.DigitalWrite(cfg_.pinAlarm, LOW);

        // Set up polling for temperature
        tedPollTemperature_.SetCallback([this](){
            OnPoll();
        });
        tedPollTemperature_.RegisterForTimedEventInterval(POLL_MS_TEMPERATURE);
        
        // Get first measurement
        tedPollTemperature_();

        // Handle events
        Log("Running");
        evm_.MainLoop();
    }


private:

    void InitLCD()
    {
        lcd_.Init();

        lcd_.PrintAt(4, 0, "Current Temp F");
        lcd_.PrintAt(0, 1, cfg_.alarmTempF);
        lcd_.PrintAt(4, 1, "Alarm   Temp F");
    }

    void OnPoll()
    {
        Log("Getting temp");
        
        SensorTemperatureDS18B20::Measurement m;
        
        if (sensorTemp_.GetMeasurement(&m))
        {
            Log("TempF: ", m.tempF);
            
            // Clear old temp and replace with new measurement
            lcd_.PrintAt(0, 0, "   ");
            lcd_.PrintAt(0, 0, m.tempF);
            
            if (m.tempF >= cfg_.alarmTempF)
            {
                AlarmOn();
            }
            else
            {
                AlarmOff();
            }
        }
        else
        {
            // Indicate error
        }
    }

    void AlarmOn()
    {
        PAL.DigitalWrite(cfg_.pinAlarm, HIGH);
    }
    
    void AlarmOff()
    {
        PAL.DigitalWrite(cfg_.pinAlarm, LOW);
    }
    
    
    
private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppThermometerAlarmConfig &cfg_;
    
    LCDFrentaly20x4 lcd_;
    
    SensorTemperatureDS18B20 sensorTemp_;
    
    TimedEventHandlerDelegate tedPollTemperature_;
};


#endif  // __APP_THERMOMETER_ALARM_H__
































