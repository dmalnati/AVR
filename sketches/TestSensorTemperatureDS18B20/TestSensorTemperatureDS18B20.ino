#include "Evm.h"
#include "SensorTemperatureDS18B20.h"


static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;

static const uint8_t PIN = 15;

static SensorTemperatureDS18B20 t(PIN);


void ReportOutcome(const char                            *type,
                   uint8_t                                retVal,
                   SensorTemperatureDS18B20::Measurement  m)
{
    if (retVal)
    {
        Serial.print(type);
        Serial.print(" ");
        Serial.println("Success -- Temperature read");

        Serial.print("tempF: ");
        Serial.println(m.tempF);
        Serial.print("tempC: ");
        Serial.println(m.tempC);
    }
    else
    {
        Serial.print(type);
        Serial.print(" ");
        Serial.println("Failure -- Could not read Temperature");
    }

    Serial.println();
}

void TestSync()
{
    SensorTemperatureDS18B20::Measurement m;

    uint8_t retVal = t.GetMeasurement(&m);

    ReportOutcome("Sync", retVal, m);
}

void TestAsync()
{
    t.GetMeasurementAsync([](SensorTemperatureDS18B20::MeasurementAsync ma){
        ReportOutcome("ASync", ma.retVal, ma.m);
        
        evm.EndMainLoop();
    });

    evm.HoldStackDangerously();
}


void setup()
{
    Serial.begin(9600);

    t.Init();

    ted.SetCallback([](){
        TestSync();
        PAL.Delay(2000);
        
        TestAsync();
        PAL.Delay(2000);
    });

    ted.RegisterForTimedEventInterval(1);

    evm.MainLoop();
}

void loop() {}




