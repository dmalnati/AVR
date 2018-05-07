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

static Evm::Instance<10,10,10> evm;
static SensorGPSUblox gps(GPS_SS_RX, GPS_SS_TX);
static SensorGPSUblox::Measurement m;
static SerialAsyncConsoleEnhanced<10,100>  console;


void Print()
{
    LogNL();
    
    Log(P("msSinceLastFix            : "), m.msSinceLastFix);
    Log(P("date                      : "), m.date);
    Log(P("time                      : "), m.time);
    Log(P("year                      : "), m.year);
    Log(P("month                     : "), m.month);
    Log(P("day                       : "), m.day);
    Log(P("hour                      : "), m.hour);
    Log(P("minute                    : "), m.minute);
    Log(P("second                    : "), m.second);
    Log(P("millisecond               : "), m.millisecond);
    Log(P("fixAge                    : "), m.fixAge);
    Log(P("courseDegrees             : "), m.courseDegrees);
    Log(P("speedKnots                : "), m.speedKnots);
    Log(P("latitudeDegreesMillionths : "), m.latitudeDegreesMillionths);
    Log(P("longitudeDegreesMillionths: "), m.longitudeDegreesMillionths);
    Log(P("latitudeDegrees           : "), m.latitudeDegrees);
    Log(P("latitudeMinutes           : "), m.latitudeMinutes);
    Log(P("latitudeSeconds           : "), m.latitudeSeconds);
    Log(P("longitudeDegrees          : "), m.longitudeDegrees);
    Log(P("longitudeMinutes          : "), m.longitudeMinutes);
    Log(P("longitudeSeconds          : "), m.longitudeSeconds);
    Log(P("altitudeFt                : "), m.altitudeFt);
    
    LogNL();
}


void setup()
{
    LogStart(9600);
    Log("Started");

    // Set up GPS
    gps.Init();

    

    console.RegisterCommand("get", [](char *){
        Log("GetMeasurement(&m)");

        uint8_t gpsLockOk = gps.GetMeasurement(&m);

        if (gpsLockOk)
        {
            Log("Locked");
        }
        else
        {
            Log("NOT Locked");
        }
    });

    console.RegisterCommand("getw", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');

            uint32_t timeoutMs = atol(p);

            Log("GetGPSLockUnderWatchdog(&m, ", timeoutMs, ")");

            uint8_t gpsLockOk = gps.GetGPSLockUnderWatchdog(&m, timeoutMs);

            if (gpsLockOk)
            {
                Log("Locked");
            }
            else
            {
                Log("NOT Locked");
            }
        }
    });

    console.RegisterCommand("get2", [](char *){
        Log("WaitForNextGPSTwoMinuteMark(&m)");

        uint8_t gpsLockOk = gps.WaitForNextGPSTwoMinuteMark(&m);

        if (gpsLockOk)
        {
            Log("Locked");
            Print();
        }
        else
        {
            Log("NOT Locked");
        }
    });

    // gets <ms> - Get new measurement, synchronously, ms timeout
    console.RegisterCommand("gets", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');

            uint32_t timeoutMs = atol(p);

            Log("GetNewMeasurementSynchronous(&m, ", timeoutMs, ", &usedMs)");

            uint32_t usedMs = 0;

            uint8_t gpsLockOk = gps.GetNewMeasurementSynchronous(&m, timeoutMs, &usedMs);

            if (gpsLockOk)
            {
                Log("Locked, usedMs: ", usedMs);
            }
            else
            {
                Log("NOT Locked, usedMs: ", usedMs);
            }
        }
    });

    console.RegisterCommand("show", [](char *){
        Print();
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

    // grid <latFloat> <lngFloat>
    console.RegisterCommand("grid", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 3)
        {
            float latFloat = atof(str.TokenAtIdx(1, ' '));
            float lngFloat = atof(str.TokenAtIdx(2, ' '));

            uint32_t latMillionths = latFloat * 1000000UL;
            uint32_t lngMillionths = lngFloat * 1000000UL;

            Log("Converting:");
            Log("  lat: ", latFloat, " -> ", latMillionths);
            Log("  lng: ", lngFloat, " -> ", lngMillionths);

            SensorGPSUblox::Measurement mTmp;

            gps.ConvertToMaidenheadGrid(latMillionths, lngMillionths, mTmp.maidenheadGrid);

            Log("  grd: ", mTmp.maidenheadGrid);
        }
    });

    

    console.Start();
    
    // handle events
    evm.MainLoop();
}

void loop() {}


