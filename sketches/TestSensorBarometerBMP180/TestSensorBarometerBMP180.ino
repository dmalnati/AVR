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
    @fix@Serial.print("tempF: ");
    @fix@Serial.print(m.tempF);
    @fix@Serial.println();

    @fix@Serial.print("tempC: ");
    @fix@Serial.print(m.tempC);
    @fix@Serial.println();

    @fix@Serial.print("pressureMilliBarAbsolute: ");
    @fix@Serial.print(m.pressureMilliBarAbsolute);
    @fix@Serial.println();

    @fix@Serial.print("pressureInchesMercuryAbsolute: ");
    @fix@Serial.print(m.pressureInchesMercuryAbsolute);
    @fix@Serial.println();

    @fix@Serial.print("pressureKiloPascalsAbsolute: ");
    @fix@Serial.print(m.pressureKiloPascalsAbsolute);
    @fix@Serial.println();

    @fix@Serial.print("pressureMilliBarSeaLevelAdjusted: ");
    @fix@Serial.print(m.pressureMilliBarSeaLevelAdjusted);
    @fix@Serial.println();

    @fix@Serial.print("pressureInchesMercurySeaLevelAdjusted: ");
    @fix@Serial.print(m.pressureInchesMercurySeaLevelAdjusted);
    @fix@Serial.println();

    @fix@Serial.print("pressureKiloPascalsSeaLevelAdjusted: ");
    @fix@Serial.print(m.pressureKiloPascalsSeaLevelAdjusted);
    @fix@Serial.println();

    @fix@Serial.print("altitudeFeetDerived: ");
    @fix@Serial.print(m.altitudeFeetDerived);
    @fix@Serial.println();
    
    @fix@Serial.print("altitudeMetersDerived: ");
    @fix@Serial.print(m.altitudeMetersDerived);
    @fix@Serial.println();
    
    
    @fix@Serial.println();
}

void setup()
{
    @fix@Serial.begin(9600);

    sensor.Init();
    
    ted.SetCallback([&]() {
        SensorBarometerBMP180::Measurement m1;
        SensorBarometerBMP180::Measurement m2;
        uint8_t retVal = 0;

        PinToggle(PIN_DEBUG_TOGGLE, 0);
        retVal = sensor.GetMeasurement(&m1);
        PinToggle(PIN_DEBUG_TOGGLE, 0);

        if (!retVal) @fix@Serial.println("Sensor read fail");
        PrintMeasurement(m1);

        PinToggle(PIN_DEBUG_TOGGLE, 0);
        retVal = sensor.GetMeasurementAndCalculatedValues(&m2, ALTITUDE_SPARKFUN_HQ_METERS);
        PinToggle(PIN_DEBUG_TOGGLE, 0);

        if (!retVal) @fix@Serial.println("Sensor read fail");
        PrintMeasurement(m2);
        
    });
    
    ted.RegisterForTimedEventInterval(DEBUG_INTERVAL_MS);
    
    evm.MainLoop();
}

void loop() {}


