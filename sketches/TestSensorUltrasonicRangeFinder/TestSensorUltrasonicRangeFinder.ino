#include "SensorUltrasonicRangeFinder.h"
#include "PAL.h"
#include "SerialLink.h"
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
        serialLink_.Init(this, &SensorMonitor::OnSerialDataReadable);

        RegisterForTimedEventInterval(150);

        evm_.MainLoop();
    }

private:
    virtual void OnTimedEvent()
    {
        // Get Measurement
        SensorUltrasonicRangeFinder::Measurement m;

        sensor_.GetMeasurement(&m);

        // Send raw data via Serial Link
        serialLink_.Send(0, &m.distIn, 1);
    }

    void OnSerialDataReadable(SerialLinkHeader *, uint8_t *, uint8_t)
    {
        // nothing to do
    }

    Evm::Instance<10,10,10> evm_;

    SerialLink<SensorMonitor, 30> serialLink_;
    SensorUltrasonicRangeFinder sensor_;
};



//static SensorMonitor mon(PIN_SENSOR_TRIGGER, PIN_SENSOR_RESPONSE);
static SensorMonitor mon(PIN_SENSOR_TRIGGER);

void setup()
{
    mon.Run();
}



void loop() {}



