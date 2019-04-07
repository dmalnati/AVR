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
static SerialAsyncConsoleEnhanced<20,100>  console;


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

    ////////////////////////////////////////////////////////////////
    //
    // Power and Initialization
    //
    ////////////////////////////////////////////////////////////////

    console.RegisterCommand("on", [](char *){
        Log(P("Turning GPS on"));
        
        console.Exec("pin set 14 1");
        console.Exec("sen out");
    });

    console.RegisterCommand("off", [](char *){
        Log(P("Turning GPS off"));
        
        console.Exec("pin set 14 0");
        console.Exec("sdis out");
    });
    
    console.RegisterCommand("init", [](char *){
        Log(P("gps.Init()"));
        gps.Init();
    });
    
    console.RegisterCommand("sen", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');

            if (!strcmp(p, "in"))
            {
                Log(P("EnableSerialInput"));
                gps.EnableSerialInput();
            }
            else if (!strcmp(p, "out"))
            {
                Log(P("EnableSerialOutput"));
                gps.EnableSerialOutput();
            }
        }
    });
    
    console.RegisterCommand("sdis", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');

            if (!strcmp(p, "in"))
            {
                Log(P("DisableSerialInput"));
                gps.DisableSerialInput();
            }
            else if (!strcmp(p, "out"))
            {
                Log(P("DisableSerialOutput"));
                gps.DisableSerialOutput();
            }
        }
    });


    ////////////////////////////////////////////////////////////////
    //
    // Displaying Data
    //
    ////////////////////////////////////////////////////////////////

    console.RegisterCommand("show", [](char *){
        Print();
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

            Log(P("Converting:"));
            Log(P("  lat: "), latFloat, P(" -> "), latMillionths);
            Log(P("  lng: "), lngFloat, P(" -> "), lngMillionths);

            SensorGPSUblox::Measurement mTmp;

            gps.ConvertToMaidenheadGrid(latMillionths, lngMillionths, mTmp.maidenheadGrid);

            Log(P("  grd: "), mTmp.maidenheadGrid);
        }
    });


    ////////////////////////////////////////////////////////////////
    //
    // Lock Breaking
    //
    ////////////////////////////////////////////////////////////////

    console.RegisterCommand("rfix", [](char *){
        Log(P("gps.ResetFix()"));
        gps.ResetFix();
    });
    
    console.RegisterCommand("rmod", [](char *){
        Log(P("ResetModule()"));
        gps.ResetModule();
    });
    

    ////////////////////////////////////////////////////////////////
    //
    // Time-related Locks
    //
    ////////////////////////////////////////////////////////////////

    /*
on
init
rfix
gettime
show
off

     */
    console.RegisterCommand("gettime", [](char *){
        Log(P("GetTimeMeasurement(&m)"));

        uint8_t gpsLockOk = gps.GetTimeMeasurement(&m);

        if (gpsLockOk)
        {
            Log(P("Time Locked"));
        }
        else
        {
            Log(P("Time NOT Locked"));
        }
    });
    
    /*
on
init
rmod
gettimesync 5000
show
off

     */
    console.RegisterCommand("gettimesync", [](char *cmdStr){
        Str str(cmdStr);

        uint32_t timeoutMs  = 5000;
        uint32_t timeUsedMs = 0;
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');

            timeoutMs = atol(p);
        }

        Log(P("GetNewTimeMeasurementSynchronous(&m, "), timeoutMs, ')');

        uint8_t gpsLockOk = gps.GetNewTimeMeasurementSynchronous(&m, timeoutMs, &timeUsedMs);

        if (gpsLockOk)
        {
            Log(P("Time Locked, ms: "), timeUsedMs);
        }
        else
        {
            Log(P("Time NOT Locked, ms: "), timeUsedMs);
        }
    });
    
    /*
on
init
rmod
gettimesyncwdt 5000
show
off

     */
    console.RegisterCommand("gettimesyncwdt", [](char *cmdStr){
        Str str(cmdStr);

        uint32_t timeoutMs  = 5000;
        uint32_t timeUsedMs = 0;
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');

            timeoutMs = atol(p);
        }

        Log(P("GetNewTimeMeasurementSynchronousUnderWatchdog(&m, "), timeoutMs, ')');
        
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_2000_MS);
        uint8_t gpsLockOk = gps.GetNewTimeMeasurementSynchronousUnderWatchdog(&m, timeoutMs, &timeUsedMs);
        PAL.WatchdogDisable();
        
        if (gpsLockOk)
        {
            Log(P("Time Locked, ms: "), timeUsedMs);
        }
        else
        {
            Log(P("Time NOT Locked, ms: "), timeUsedMs);
        }
    });
    
    /*
on
init
rmod
gettimesyncwdt2
show
off

     */
    console.RegisterCommand("gettimesyncwdt2", [](char *){
        Log(P("GetNewTimeMeasurementSynchronousTwoMinuteMarkUnderWatchdog(&m)"));

        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_2000_MS);
        uint8_t gpsLockOk = gps.GetNewTimeMeasurementSynchronousTwoMinuteMarkUnderWatchdog(&m);
        PAL.WatchdogDisable();

        if (gpsLockOk)
        {
            Log(P("Time Locked"));
        }
        else
        {
            Log(P("Time NOT Locked"));
        }
    });


    ////////////////////////////////////////////////////////////////
    //
    // Location Lock
    //
    ////////////////////////////////////////////////////////////////
    
    /*
on
init
rfix
getloc
show
off

     */
    console.RegisterCommand("getloc", [](char *){
        Log(P("GetLocationMeasurement(&m)"));

        uint8_t gpsLockOk = gps.GetLocationMeasurement(&m);

        if (gpsLockOk)
        {
            Log(P("Location Locked"));
        }
        else
        {
            Log(P("Location NOT Locked"));
        }
    });
    
    /*
on
init
rmod
getlocsync 5000
show
off

     */
    console.RegisterCommand("getlocsync", [](char *cmdStr){
        Str str(cmdStr);
        
        uint32_t timeoutMs  = 5000;
        uint32_t timeUsedMs = 0;
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');

            timeoutMs = atol(p);
        }

        Log(P("GetNewLocationMeasurementSynchronous(&m, "), timeoutMs, ')');

        uint8_t gpsLockOk = gps.GetNewLocationMeasurementSynchronous(&m, timeoutMs, &timeUsedMs);

        if (gpsLockOk)
        {
            Log(P("Location Locked, ms: "), timeUsedMs);
        }
        else
        {
            Log(P("Location NOT Locked, ms: "), timeUsedMs);
        }
    });
    
    /*
on
init
rmod
getlocsyncwdt 5000
show
off

     */
    console.RegisterCommand("getlocsyncwdt", [](char *cmdStr){
        Str str(cmdStr);

        uint32_t timeoutMs  = 5000;
        uint32_t timeUsedMs = 0;
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');

            timeoutMs = atol(p);
        }

        Log(P("GetNewLocationMeasurementSynchronousUnderWatchdog(&m, "), timeoutMs, ')');
        
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_2000_MS);
        uint8_t gpsLockOk = gps.GetNewLocationMeasurementSynchronousUnderWatchdog(&m, timeoutMs, &timeUsedMs);
        PAL.WatchdogDisable();
        
        if (gpsLockOk)
        {
            Log(P("Location Locked, ms: "), timeUsedMs);
        }
        else
        {
            Log(P("Location NOT Locked, ms: "), timeUsedMs);
        }
    });



    ////////////////////////////////////////////////////////////////
    //
    // Startup
    //
    ////////////////////////////////////////////////////////////////

    console.Start();
    console.Exec("off");
    
    // handle events
    evm.MainLoop();
}

void loop() {}


