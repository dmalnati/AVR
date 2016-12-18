#include "SensorUltrasonicRangeFinder.h"
#include "PAL.h"
#include "Evm.h"
#include "TimedEventHandler.h"


static const uint8_t PIN_SENSOR_TRIGGER  = 14;
static const uint8_t PIN_SENSOR_RESPONSE = 15;


class SensorMonitor
: public TimedEventHandler
{
public:
    SensorMonitor(uint8_t pinTrigger, uint8_t pinResponse)
    : sensor_(pinTrigger, pinResponse)
    {
        // Nothing to do
    }

    SensorMonitor(uint8_t pinTriggerAndResponse)
    : sensor_(pinTriggerAndResponse)
    {
        // Nothing to do
    }

    void Run()
    {
        Serial.begin(9600);
    
        RegisterForTimedEventInterval(500);

        evm_.MainLoop();
    }

private:
    virtual void OnTimedEvent()
    {
        // Get Measurement
        SensorUltrasonicRangeFinder::Measurement m;

        sensor_.GetMeasurement(&m);

        Serial.print("Inches: ");
        Serial.println(m.distIn);
    }

    Evm::Instance<10,10,10> evm_;

    SensorUltrasonicRangeFinder sensor_;
};



//static SensorMonitor mon(PIN_SENSOR_TRIGGER, PIN_SENSOR_RESPONSE);
static SensorMonitor mon(PIN_SENSOR_TRIGGER);

void setup()
{
    mon.Run();
}



void loop() {}



