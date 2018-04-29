#define IVM_DISABLE

#include "Software@fix@Serial.h"

#include "PAL.h"
#include "Evm.h"
#include "SensorGPSUblox.h"


static const uint32_t DEBUG_INTERVAL_MS = 2000;
static const uint32_t DEBUG_BAUD        = 9600;

static const int8_t PIN_GPS_RX = 5;
static const int8_t PIN_GPS_TX = 6;

static const int8_t PIN_DEBUG_RX = -1;
static const int8_t PIN_DEBUG_TX = 4;


static SoftwareSerial ss(PAL.GetArduinoPinFromPhysicalPin(PIN_DEBUG_RX),
                         PAL.GetArduinoPinFromPhysicalPin(PIN_DEBUG_TX));

static Evm::Instance<10,10,10> evm;

static TimedEventHandlerDelegate ted;
static SensorGPSUblox gps(PIN_GPS_RX, PIN_GPS_TX);


void setup()
{
    ss.begin(DEBUG_BAUD);
    ss.println("Started");
    
    gps.Init();
    ss.println("GPS Init'd");
    
    ted.SetCallback([&]() {
        SensorGPSUblox::Measurement m;

        if (gps.GetMeasurement(&m))
        {
            ss.println("Got Measurement");
            
            ss.print("    msSinceLastFix: ");
            ss.print(m.msSinceLastFix);
            ss.println();
            
            ss.print("    date          : ");
            ss.print(m.date);
            ss.println();
            
            ss.print("    time          : ");
            ss.print(m.time);
            ss.println();
            
            ss.print("    latitude      : ");
            ss.print(m.latitude);
            ss.println();
            
            ss.print("    longitude     : ");
            ss.print(m.longitude);
            ss.println();
            
            ss.print("    altitude      : ");
            ss.print(m.altitude);
            ss.println();

            ss.println();
        }
        else
        {
            ss.println("Measurement Failed");
        }
    });
    ss.println("Set Callback");
    
    ted.RegisterForTimedEventInterval(DEBUG_INTERVAL_MS);
    ss.println("Timer Set");
    
    evm.MainLoop();
}

void loop() {}


