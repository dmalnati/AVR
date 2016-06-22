#include "Evm.h"
#include "Utl.h"
#include "SensorEcompassLSM303C.h"


static const uint32_t DEBUG_INTERVAL_MS = 1000;
static const uint8_t  PIN_DEBUG_TOGGLE = 4;

static Evm::Instance<10,10,10> evm;

static TimedEventHandlerDelegate ted;
static SensorEcompassLSM303C sensor;

void PrintMeasurement(SensorEcompassLSM303C::Measurement m)
{
    Serial.print("accelX: ");
    Serial.print(m.accelX);
    Serial.println();

    Serial.print("accelY: ");
    Serial.print(m.accelY);
    Serial.println();
    
    Serial.print("accelZ: ");
    Serial.print(m.accelZ);
    Serial.println();
    
    Serial.print("magX: ");
    Serial.print(m.magX);
    Serial.println();

    Serial.print("magY: ");
    Serial.print(m.magY);
    Serial.println();

    Serial.print("magZ: ");
    Serial.print(m.magZ);
    Serial.println();
    
    Serial.print("tempF: ");
    Serial.print(m.tempF);
    Serial.println();

    Serial.println();
}

void setup()
{
    Serial.begin(9600);

    sensor.Init();
    
    ted.SetCallback([&]() {
        SensorEcompassLSM303C::Measurement m;

        PinToggle(PIN_DEBUG_TOGGLE, 0);
        sensor.GetMeasurement(&m);
        PinToggle(PIN_DEBUG_TOGGLE, 0);

        PrintMeasurement(m);
    });
    
    ted.RegisterForTimedEventInterval(DEBUG_INTERVAL_MS);
    
    evm.MainLoop();
}

void loop() {}


