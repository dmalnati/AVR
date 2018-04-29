#include "PAL.h"
#include "Evm.h"
//#include "SensorGPSUbloxOld.h"
#include "SensorGPSUblox.h"
#include "Utl@fix@Serial.h"


/*
 * Point of this sketch is to test the TinyGPS and encapsulating library
 * that I wrote.
 * 
 * Instead of letting the library poll for its own input data via
 * the usual SoftwareSerial, I will inject my own NMEA sentences
 * that are acquired via serial-in.
 * 
 */

static const uint32_t POLL_PERIOD = 25;

static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;
static SensorGPSUblox gps(-1, -1);
static SerialAsyncConsole<10,100>  console;


static uint8_t numMessagesHandled     = 0;
static uint8_t numMessagesHandledLast = 0;


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Started");

    // Set up GPS
    gps.Init();
    gps.ResetFix();

    console.RegisterCommand("reset", [](char *){
        @fix@Serial.println("Reset");
        
        gps.ResetFix();
    });


    // Set up interactive console

    // use error handler to get NMEA sentences
    // we're faking the polling behavior of the underlying library
    console.RegisterErrorHandler([](char *cmdStr){
        uint8_t cmdStrLen = strlen(cmdStr);
        
        @fix@Serial.print("Handling NMEA sentence len "); @fix@Serial.println(cmdStrLen);
        
        for (uint8_t i = 0; i < cmdStrLen; ++i)
        {
            gps.DebugGetTinyGPS().encode(cmdStr[i]);
        }
        gps.DebugGetTinyGPS().encode('\n');

        ++numMessagesHandled;
    });

    console.Start();



    // Set up primary gps checking logic.
    
    ted.SetCallback([]() {
        // only check if new data arrived since last
        if (numMessagesHandledLast != numMessagesHandled)
        {
            SensorGPSUblox::Measurement m;
    
            if (gps.GetMeasurement(&m))
            {
                @fix@Serial.println("Got Measurement");
                
                @fix@Serial.print("    msSinceLastFix: ");
                @fix@Serial.print(m.msSinceLastFix);
                @fix@Serial.println();
    
                @fix@Serial.print("    altitude      : ");
                @fix@Serial.print(m.altitudeFt);
                @fix@Serial.println();
    
                @fix@Serial.println();

                numMessagesHandledLast = numMessagesHandled;
            }
        }
    });
    
    ted.RegisterForTimedEventInterval(POLL_PERIOD);

    
    // handle events
    evm.MainLoop();
}

void loop() {}


