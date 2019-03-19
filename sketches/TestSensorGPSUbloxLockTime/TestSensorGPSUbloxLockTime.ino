#include "Log.h"
#include "LogBlob.h"
#include "Evm.h"
#include "SoftwareSerial.h"
#include "SerialInput.h"
#include "SensorGPSUblox.h"




/*
 * Purpose is to gather information about the lock time of a GPS.
 * 
 * I want to test several things:
 * - Power GPS on, lock, power off (no battery backup)
 *   - Do this N times, reporting on each
 * - Power GPS on, lock, power off (with battery backup)
 *   - Do this N times, reporting on each
 * 
 * User controls:
 * - select mode
 * - start [<N>]
 * - stop
 * 
 */






// Not actually used, just needs a concrete instance
// to be here to safely instantiate any objects (gps)
// which use the evm behind the scenes.
static Evm::Instance<0,5,0> evm;

static const uint8_t PIN_GPS_ENABLE = 14;
static const int8_t PIN_GPS_RX = 23;
static const int8_t PIN_GPS_TX = 24;
static SensorGPSUblox gps(PIN_GPS_RX, PIN_GPS_TX);
static SensorGPSUblox::Measurement gpsMeasurement;

static UbxMessage<100> ubxMsg;

TimedEventHandlerDelegate tedWaitForGpsLock;

TimedEventHandlerDelegate tedBatch;

static SoftwareSerial &ss = gps.DebugGetSS();

static const uint8_t NUM_COMMANDS = 16;
static SerialAsyncConsoleEnhanced<NUM_COMMANDS>  shell;




static uint8_t RUNNING = 0;










// Copied and adjusted from WSPR tracker
    void StartSubsystemGPS()
    {
        PAL.DigitalWrite(PIN_GPS_ENABLE, HIGH);
        
        gps.EnableSerialInput();
        
        gps.EnableSerialOutput();
    }
    
    void StopSubsystemGPS()
    {
        // Work around a hardware issue where the GPS will draw lots of current
        // through a logic input
        gps.DisableSerialOutput();
        
        // disable power supply to GPS
        // (battery backup for module-stored data supplied through other pin)
        PAL.DigitalWrite(PIN_GPS_ENABLE, LOW);
        
        // stop interrupts from firing in underlying code
        gps.DisableSerialInput();
    }
    
    void StartGPS()
    {
        // Enable subsystem
        StartSubsystemGPS();
        
        // re-init to begin cycle again
        gps.Init();
        
        // assert this is a high-altitude mode
        gps.SetHighAltitudeMode();
        
        // Only care about two messages, indicate so
        gps.EnableOnlyGGAAndRMC();
    }

    void StopGPS(uint8_t saveConfiguration = 0)
    {
        // Don't allow prior fix to be used, we want a brand new fix to be
        // acquired next time the GPS starts up
        gps.ResetFix();

        // cause the gps module to store the metadata is has learned from
        // the satellites it can see and used to get a lock.
        // this will be read again automatically by the module on startup.
        if (saveConfiguration)
        {
            gps.SaveConfiguration();
        }
        
        // Disable subsystem
        StopSubsystemGPS();
    }







/*
 * Todo
 * - support batch mode
 *   eg do batt for a while, then nobatt
 *     better to run overnight, and then you're collecting lots of samples for both
 */






uint32_t DELAY_MS = 15000;

enum class Mode : uint8_t
{
    BATT,
    NO_BATT,
};

Mode mode = Mode::BATT;


uint32_t TIME_START = 0;
uint32_t TIME_LAST = 0;
void OnCheckForLock()
{
    // Check if GPS is locked on, saving the result in the process
    if (gps.GetMeasurement(&gpsMeasurement))
    {
        // Cancel task to check for GPS lock
        tedWaitForGpsLock.DeRegisterForTimedEvent();
        
        // Notify that lock acquired
        uint32_t timeNow      = PAL.Millis();
        uint32_t msSinceStart = timeNow - TIME_START;
        uint32_t msGpsLock    = timeNow - TIME_LAST;
        
        Log(msSinceStart, ", ", msGpsLock / 1000);

        // Start next lock sequence
        Schedule();
    }
}

void Schedule()
{
    if (mode == Mode::BATT)
    {
        uint8_t saveConfiguration = 1;
        StopGPS(saveConfiguration);

        if (RUNNING)
        {
            PAL.Delay(DELAY_MS);
        }

        StartGPS();
    }
    else
    {
        // Command to forget prior gps lock data
        if (RUNNING)
        {
            shell.Exec("resetgps cold");
        }

        uint8_t saveConfiguration = 0;
        StopGPS(saveConfiguration);
        
        if (RUNNING)
        {
            PAL.Delay(DELAY_MS);
        }
        
        StartGPS();
    }

    TIME_LAST = PAL.Millis();
    if (TIME_START == 0)
    {
        TIME_START = PAL.Millis();
    }
    tedWaitForGpsLock.SetCallback([](){ OnCheckForLock(); });
    tedWaitForGpsLock.RegisterForTimedEventInterval(1000);
}

void Stop()
{
    tedWaitForGpsLock.DeRegisterForTimedEvent();

    if (RUNNING)
    {
        shell.Exec("resetgps cold");
    }

    uint8_t saveConfiguration = 0;
    StopGPS(saveConfiguration);

    TIME_START = 0;
}

void BStart(uint32_t durationMs)
{
    static Mode batchMode = Mode::BATT;
    
    BStop();

    batchMode = Mode::BATT;
    tedBatch.SetCallback([&](){
        LogNL();
        shell.Exec("stop");
        
        if (batchMode == Mode::BATT)
        {
            shell.Exec("mode nobatt");
            batchMode = Mode::NO_BATT;
        }
        else
        {
            shell.Exec("mode batt");
            batchMode = Mode::BATT;
        }
        
        shell.Exec("start");
    });

    shell.Exec("start");
    tedBatch.RegisterForTimedEventInterval(durationMs);
}

void BStop()
{
    shell.Exec("stop 0");

    tedBatch.DeRegisterForTimedEvent();
}




void PrintMenu()
{
    LogNL(2);
    Log(P("GPS Lock Time Tester"));
    Log(P("--------------------"));
    LogNL();
    Log(P("mode <arg> - either 'batt' or 'nobatt'"));
    Log(P("  batt   - make use of battery backup between start/stop of GPS"));
    Log(P("  nobatt - no battery backup, explicit memory clear between start/stop of GPS"));
    Log(P("delay <arg> - duration of idle time between GPS locks, in ms"));
    Log(P("start - begin testing"));
    Log(P("stop - cease testing"));
    Log(P("bstart <arg> - batch start, do one mode for <arg> min, then the other, forever"));
    Log(P("bstop - stop batch"));
    LogNL();
    Log(P("When running, a continuous cycle of:"));
    Log(P("- Start GPS"));
    Log(P("- Get GPS lock"));
    Log(P("- Print time to acquire lock"));
    Log(P("- Stop GPS"));
    LogNL();
}


void setup()
{
    LogStart(9600);


    shell.RegisterCommand("mode", 1, [](char *cmdStr) {
        Str str(cmdStr);

        uint8_t ok = 1;
        
        if (!strcmp("batt", str.TokenAtIdx(1, ' ')))
        {
            Log("Battery mode");
            mode = Mode::BATT;
        }
        else if (!strcmp("nobatt", str.TokenAtIdx(1, ' ')))
        {
            Log("No-Battery mode");
            mode = Mode::NO_BATT;
        }
        else
        {
            ok = 0;
        }

        if (ok)
        {
            if (RUNNING)
            {
                Log("Re-Starting since already running");
                
                shell.Exec("start");
            }
        }
    });

    shell.RegisterCommand("delay", 1, [](char *cmdStr) {
        Str str(cmdStr);
        
        DELAY_MS = atol(str.TokenAtIdx(1, ' '));

        Log(P("Setting delay to "), DELAY_MS);
    });

    shell.RegisterCommand("start", [](char *) {
        shell.Exec("stop 0");
        
        Log("Starting [mode=",
             mode == Mode::BATT ? "BATT" : "NO_BATT",
             ", delay=",
             DELAY_MS,
             "]");
        LogNL();
        Log("time, durationSecs");

        Schedule();

        RUNNING = 1;
    });

    // allows calling with a secret 0 parameter which suppresses printout
    shell.RegisterCommand("stop", [](char *cmdStr) {
        Str str(cmdStr);

        uint8_t verbose = 1;
        if (str.TokenCount(' ') >= 2)
        {
            verbose = atoi(str.TokenAtIdx(2, ' '));
        }

        if (verbose)
        {
            Log("Stopping");
        }

        Stop();

        if (verbose)
        {
            LogNL();
        }

        RUNNING = 0;
    });






    shell.RegisterCommand("bstart", [](char *cmdStr) {
        Str str(cmdStr);
        
        uint32_t durationMinutes = 30;

        if (str.TokenCount(' ') >= 2)
        {
            durationMinutes = atoi(str.TokenAtIdx(1, ' '));
        }

        uint32_t durationMs = durationMinutes * 60 * 1000;

        Log(P("Batch Start, each "), durationMinutes, " min (", durationMinutes / 60, " hours)");

        BStart(durationMs);
    });


    shell.RegisterCommand("bstop", [](char *) {
        Log(P("Batch Stop"));

        BStop();
    });


    shell.RegisterCommand("resetgps", [](char *cmdStr) {
        Str str(cmdStr);

        uint16_t resetType = 0x0000;
        if (str.TokenCount(' ') >= 2)
        {
            const char *resetTypeStr = str.TokenAtIdx(1, ' ');
            
            if      (!strcmp(resetTypeStr, "hot"))  { resetType = 0x0000; }
            else if (!strcmp(resetTypeStr, "warm")) { resetType = 0x0001; }
            else if (!strcmp(resetTypeStr, "cold")) { resetType = 0xFFFF; }
        }

        uint8_t resetMode = 0x00;
        if (str.TokenCount(' ') >= 3)
        {
            const char *resetModeStr = str.TokenAtIdx(2, ' ');
            
            if      (!strcmp(resetModeStr, "hw"))                  { resetMode = 0x00; }
            else if (!strcmp(resetModeStr, "sw"))                  { resetMode = 0x01; }
            else if (!strcmp(resetModeStr, "sw_gps_only"))         { resetMode = 0x02; }
            else if (!strcmp(resetModeStr, "hw_after_sd"))         { resetMode = 0x04; }
            else if (!strcmp(resetModeStr, "controlled_gps_down")) { resetMode = 0x08; }
            else if (!strcmp(resetModeStr, "controlled_gps_up"))   { resetMode = 0x09; }
        }

        //Log(P("Resetting resetType: "), resetType, P(", resetMode: "), resetMode);

        // Fill out UBX Message
        ubxMsg.Reset();

        // CFG-RST (0x06 0x04)
        ubxMsg.SetClass(0x06);
        ubxMsg.SetId(0x04);

        ubxMsg.AddFieldX2(resetType);
        ubxMsg.AddFieldU1(resetMode);
        ubxMsg.AddFieldU1(0);   // reserved

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        //LogBlob(buf, bufLen, 0, 1);
        ss.write(buf, bufLen);
    });


    
    shell.RegisterCommand("save", [](char *) {
        ubxMsg.Reset();

        // CFG-CFG (0x06 0x09)
        ubxMsg.SetClass(0x06);
        ubxMsg.SetId(0x09);

        ubxMsg.AddFieldX4(0);           // clearMask  - clear nothing
        ubxMsg.AddFieldX4(0x0000FFFF);  // saveMask   - save everything
        ubxMsg.AddFieldX4(0);           // loadMask   - load nothing
        ubxMsg.AddFieldX1(1);           // deviceMask - save to batter-backed ram (internal to chip)

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        LogBlob(buf, bufLen, 0, 1);
        ss.write(buf, bufLen);
    });

    
    shell.SetVerbose(0);
    shell.Start();

    PrintMenu();

    evm.MainLoop();
}

void loop() {}




















