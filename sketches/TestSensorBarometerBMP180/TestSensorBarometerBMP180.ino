#include "Evm.h"
#include "Utl.h"
#include "SensorBarometerBMP180.h"


static const uint32_t DEBUG_INTERVAL_MS = 1000;
static const uint8_t  PIN_DEBUG_TOGGLE = 4;
constexpr static const double ALTITUDE_HOBOKEN_METERS = 7.900416;
constexpr static const double ALTITUDE_SPARKFUN_HQ_METERS =  1655.0; // Altitude of SparkFun's HQ in Boulder, CO. in meters

static Evm::Instance<10,10,10> evm;

static TimedEventHandlerDelegate ted;
static SensorBarometerBMP180 sensor;

void PrintMeasurement(SensorBarometerBMP180::Measurement m)
{
    Serial.print("tempF: ");
    Serial.print(m.tempF);
    Serial.println();

    Serial.print("tempC: ");
    Serial.print(m.tempC);
    Serial.println();

    Serial.print("pressureMilliBarAbsolute: ");
    Serial.print(m.pressureMilliBarAbsolute);
    Serial.println();

    Serial.print("pressureInchesMercuryAbsolute: ");
    Serial.print(m.pressureInchesMercuryAbsolute);
    Serial.println();

    Serial.print("pressureKiloPascalsAbsolute: ");
    Serial.print(m.pressureKiloPascalsAbsolute);
    Serial.println();

    Serial.print("pressureMilliBarSeaLevelAdjusted: ");
    Serial.print(m.pressureMilliBarSeaLevelAdjusted);
    Serial.println();

    Serial.print("pressureInchesMercurySeaLevelAdjusted: ");
    Serial.print(m.pressureInchesMercurySeaLevelAdjusted);
    Serial.println();

    Serial.print("pressureKiloPascalsSeaLevelAdjusted: ");
    Serial.print(m.pressureKiloPascalsSeaLevelAdjusted);
    Serial.println();

    Serial.print("altitudeFeetDerived: ");
    Serial.print(m.altitudeFeetDerived);
    Serial.println();
    
    Serial.print("altitudeMetersDerived: ");
    Serial.print(m.altitudeMetersDerived);
    Serial.println();
    
    
    Serial.println();
}

void setup()
{
    Serial.begin(9600);

    sensor.Init();
    
    ted.SetCallback([&]() {
        SensorBarometerBMP180::Measurement m1;
        SensorBarometerBMP180::Measurement m2;
        uint8_t retVal = 0;

        PinToggle(PIN_DEBUG_TOGGLE, 0);
        retVal = sensor.GetMeasurement(&m1);
        PinToggle(PIN_DEBUG_TOGGLE, 0);

        if (!retVal) Serial.println("Sensor read fail");
        PrintMeasurement(m1);

        PinToggle(PIN_DEBUG_TOGGLE, 0);
        retVal = sensor.GetMeasurementAndCalculatedValues(&m2, ALTITUDE_SPARKFUN_HQ_METERS);
        PinToggle(PIN_DEBUG_TOGGLE, 0);

        if (!retVal) Serial.println("Sensor read fail");
        PrintMeasurement(m2);
        
    });
    
    ted.RegisterForTimedEventInterval(DEBUG_INTERVAL_MS);
    
    evm.MainLoop();
}

void loop() {}


