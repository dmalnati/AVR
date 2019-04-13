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

static Evm::Instance<0,5,0> evm;
static SensorGPSUblox gps(GPS_SS_RX, GPS_SS_TX);
static SensorGPSUblox::Measurement m;
static SerialAsyncConsoleEnhanced<20,20>  console;












////////////////////////////////////////////////////////////////
//
// Unit Tests
//
////////////////////////////////////////////////////////////////


// A number of time-lock functions use only a subset of the Measurement fields.
// We mock them up for test scenarios.

struct TimeTestScenario
{
    uint32_t clockTimeAtMeasurement;
    uint8_t  minute;
    uint8_t  second;
    uint16_t millisecond;
    
    uint32_t expectedResult;
};

SensorGPSUblox::Measurement GetMeasurementFromTimeTestScenario(TimeTestScenario scenario)
{
    SensorGPSUblox::Measurement m;
    
    m.clockTimeAtMeasurement = scenario.clockTimeAtMeasurement;
    m.minute                 = scenario.minute;
    m.second                 = scenario.second;
    m.millisecond            = scenario.millisecond;
    
    return m;
}

void TestCalculateDurationMsToNextTwoMinuteMark()
{
    // Testing SleepToCloseToTwoMinMark
    //
    // Pass in:
    // - a GPS time lock object
    //   - so the time fields set, and the clockTimeAtMeasurement set
    // - a desired duration to wake up before the 2 min mark of that lock
    //
    // It returns:
    // - the duration of time between the time of measurement and the next
    //   two minute mark.
    
    uint32_t timeNow = PAL.Millis();
    
    TimeTestScenario scenarioList[] =
    {
        { timeNow, 0, 1, 400, 118600 }, // 00:01.400 = 118600
        { timeNow, 1, 1, 400,  58600 }, // 01:01.400 =  58600
    };
    
    Log(P("TestCalculateDurationMsToNextTwoMinuteMark"));
    
    for ( auto &scenario : scenarioList )
    {
        SensorGPSUblox::Measurement m = GetMeasurementFromTimeTestScenario(scenario);

        uint32_t result = gps.CalculateDurationMsToNextTwoMinuteMark(&m);
        
        printf("%02i:%02i.%03i - ", m.minute, m.second, m.millisecond);
        if (result == scenario.expectedResult)
        {
            Log(P("OK"));
        }
        else
        {
            Log(P("NOT OK"));
        }
    }
}

void TestSleepToCloseToTwoMinMark()
{
    // Testing SleepToCloseToTwoMinMark
    //
    // Pass in:
    // - Time measurement
    // - how long should be remaining, in real clock time, before the 2 min mark
    //   when the function returns.
    //
    // It returns:
    // - the actual duration of time remaining before the 2 min mark
    
    // Assume we want to wake up 5000ms before mark
    const uint32_t DURATION_TARGET_BEFORE_TWO_MIN_MARK_TO_WAKE_MS = 5000;
    TimeTestScenario scenarioList[] =
    {
        {    0, 1, 55, 0, 5000 },    //  5 sec before mark, shouldn't sleep, should return 5000
        { 1000, 1, 55, 0, 4000 },    //  5 sec before mark, shouldn't sleep, perceives delay by 1 sec, should return 4000
        { 6000, 1, 55, 0,    0 },    //  5 sec before mark, shouldn't sleep, perceives delay by 6 sec, should return 0
        {    0, 1, 56, 0, 4000 },    //  4 sec before mark, shouldn't sleep, should return 4000
        {    0, 1, 53, 0, 5000 },    //  7 sec before mark, should sleep 2, should return 5000
        { 3000, 1, 53, 0, 4000 },    //  7 sec before mark, should sleep 2, but perceives delay by 3000, should return 4000
        { 8000, 1, 53, 0,    0 },    //  7 sec before mark, should sleep 2, but perceives delay by 8000, should return 0
    };
    
    Log(P("TestSleepToCloseToTwoMinMark"));
    
    uint8_t count = 1;
    for (auto &scenario : scenarioList)
    {
        Log(P("Test "), count);
        ++count;
        
        SensorGPSUblox::Measurement m = GetMeasurementFromTimeTestScenario(scenario);

        // pretend the measurement was taken right now, or purposefully delay,
        // but tight control is necessary since clock drift is
        // an important part of this code
        uint32_t purposefulDelayInClock = m.clockTimeAtMeasurement;
        m.clockTimeAtMeasurement = PAL.Millis();
        PAL.Delay(purposefulDelayInClock);
        
        uint32_t result = gps.SleepToCloseToTwoMinMark(&m, DURATION_TARGET_BEFORE_TWO_MIN_MARK_TO_WAKE_MS);
        
        printf("%02i:%02i.%03i - ", m.minute, m.second, m.millisecond);
        LogNNL("e(", scenario.expectedResult, P(") ~= a("), result, P(")? - "));
        if (abs(result - scenario.expectedResult) <= 3)
        {
            Log(P("OK"));
        }
        else
        {
            Log(P("NOT OK"));
        }
    }
}


////////////////////////////////////////////////////////////////
//
// Helper functions
//
////////////////////////////////////////////////////////////////


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


















////////////////////////////////////////////////////////////////
//
// Main code
//
////////////////////////////////////////////////////////////////




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
        
        const uint32_t DURATION_MAX_GPS_TIME_LOCK_WAIT_MS = 5000;
        
        // This should be the durationForInitialLock, which calling code can decide,
        // but for the tracker probably say the the giveup time associated with the
        // regular GPS location lock.
        
        function<void(void)> fnBeforeAttempt = [](){
            console.Exec("on");
        };
        
        function<void(void)> fnAfterAttempt = [](){
            console.Exec("off");
        };
        
        function<uint8_t(void)> fnOkToContinue = [](){
            return 1;
        };
        
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_8000_MS);
        uint8_t gpsLockOk =
            gps.GetNewTimeMeasurementSynchronousTwoMinuteMarkUnderWatchdog(
                &m,
                 DURATION_MAX_GPS_TIME_LOCK_WAIT_MS,
                 fnBeforeAttempt,
                 fnAfterAttempt,
                 fnOkToContinue);
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
    // Unit Testing
    //
    ////////////////////////////////////////////////////////////////

    console.RegisterCommand("utest", [](char *){
        TestCalculateDurationMsToNextTwoMinuteMark();
        TestSleepToCloseToTwoMinMark();
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








































