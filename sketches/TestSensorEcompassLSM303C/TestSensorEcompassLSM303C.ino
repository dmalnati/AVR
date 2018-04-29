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
    @fix@Serial.print("accelX: ");
    @fix@Serial.print(m.accelX);
    @fix@Serial.println();

    @fix@Serial.print("accelY: ");
    @fix@Serial.print(m.accelY);
    @fix@Serial.println();
    
    @fix@Serial.print("accelZ: ");
    @fix@Serial.print(m.accelZ);
    @fix@Serial.println();
    
    @fix@Serial.print("magX: ");
    @fix@Serial.print(m.magX);
    @fix@Serial.println();

    @fix@Serial.print("magY: ");
    @fix@Serial.print(m.magY);
    @fix@Serial.println();

    @fix@Serial.print("magZ: ");
    @fix@Serial.print(m.magZ);
    @fix@Serial.println();
    
    @fix@Serial.print("tempF: ");
    @fix@Serial.print(m.tempF);
    @fix@Serial.println();

    @fix@Serial.println();
}

void setup()
{
    @fix@Serial.begin(9600);

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


