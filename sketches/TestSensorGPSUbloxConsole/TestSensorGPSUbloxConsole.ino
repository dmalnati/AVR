#include "PAL.h"
#include "SerialInput.h"
#include "Evm.h"
#include "SensorGPSUblox.h"
#include "Log.h"


/*
 * Interactive control over library interface while using real GPS device.
 */

static const uint8_t GPS_SS_RX = 23;
static const uint8_t GPS_SS_TX = 24;
//static const uint8_t GPS_SS_TX = 9;

static Evm::Instance<10,10,10> evm;
static SensorGPSUblox gps(GPS_SS_RX, GPS_SS_TX);
static SerialAsyncConsoleEnhanced<10,100>  console;



void setup()
{
    LogStart(9600);
    Log("Started");

    // Set up GPS
    gps.Init();

    

    // gets <ms> - Get new measurement, synchronously, ms timeout
    console.RegisterCommand("gets", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');

            uint32_t timeoutMs = atol(p);

            Log("GetNewMeasurementSynchronous(&m, ", timeoutMs, ", &usedMs)");

            uint32_t usedMs = 0;
            SensorGPSUblox::Measurement m;
            uint8_t gpsLockOk = gps.GetNewMeasurementSynchronous(&m, timeoutMs, &usedMs);

            if (gpsLockOk)
            {
                Log("Locked, usedMs: ", usedMs);
            }
            else
            {
                Log("NOT Locked");
            }
        }
    });

    console.RegisterCommand("enable", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');

            if (!strcmp(p, "in"))
            {
                Log("EnableSerialInput");
                gps.EnableSerialInput();
            }
            else if (!strcmp(p, "out"))
            {
                Log("EnableSerialOutput");
                gps.EnableSerialOutput();
            }
        }
    });
    
    console.RegisterCommand("disable", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');

            if (!strcmp(p, "in"))
            {
                Log("DisableSerialInput");
                gps.DisableSerialInput();
            }
            else if (!strcmp(p, "out"))
            {
                Log("DisableSerialOutput");
                gps.DisableSerialOutput();
            }
        }
    });

    console.Start();
    
    // handle events
    evm.MainLoop();
}

void loop() {}


