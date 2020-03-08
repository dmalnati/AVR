#include "PAL.h"
#include "SerialInput.h"
#include "Evm.h"
#include "SensorTemperatureMCP9808.h"
#include "Log.h"



static Evm::Instance<0,5,0> evm;
static SerialAsyncConsoleEnhanced<20,20>  console;
static SensorTemperatureMCP9808 sensor;



void Display()
{
    SensorTemperatureMCP9808::Measurement m;

    uint8_t retVal = sensor.GetMeasurement(&m);

    Log(P("Success: "), retVal);

    Log(P("manufacturerId: "), m.manufacturerId);
    Log(P("deviceId: "), m.deviceId);
    Log(P("deviceRevision: "), m.deviceRevision);
    

    Log(P("Configuration:"));
    Log(P("  tHyst: "),     m.cfg.tHyst);
    Log(P("  shdn: "),      m.cfg.shdn);
    Log(P("  critLock: "),  m.cfg.critLock);
    Log(P("  winLock: "),   m.cfg.winLock);
    Log(P("  intClear: "),  m.cfg.intClear);
    Log(P("  alertStat: "), m.cfg.alertStat);
    Log(P("  alertCntl: "), m.cfg.alertCntl);
    Log(P("  alertSel: "),  m.cfg.alertSel);
    Log(P("  alertPol: "),  m.cfg.alertPol);
    Log(P("  alertMod: "),  m.cfg.alertMod);


    Log(P("SensorResolution: "), m.sensorResolution);
    Log(P("TempF: "), m.tempF);
    Log(P("TempC: "), m.tempC);

    Log(P("TempAlertUpper: "), m.tempAlertUpper);
    Log(P("TempAlertLower: "), m.tempAlertLower);
    Log(P("TempCritical: "), m.tempCritical);
}

void setup()
{
    LogStart(9600);
    Log("Starting");
    
    console.RegisterCommand("disp", [](char *){
        Log(P("Displaying measurement"));
        
        Display();
    });
    
    console.RegisterCommand("setres", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint8_t val = atoi(str.TokenAtIdx(1, ' '));

            Log(P("Setting sensor resolution to: "), val);
            sensor.SetSensorResolution(val);

            Display();
        }
    });

    // my meter reads 0.0uA after this
    console.RegisterCommand("sleep", [](char *){
        Log(P("Sleeping"));
        
        sensor.Sleep();
    });

    // my meter reads ~170uA after this
    console.RegisterCommand("wake", [](char *){
        Log(P("Waking"));
        
        sensor.Wake();
    });

    // always zero, asleep or awake
    // doesn't work when sleeping, as in, temperature never changes
    console.RegisterCommand("ttemp", [](char *){
        Log(P("Timing a temperature reading"));
        
        uint32_t timeNow = PAL.Millis();
        int8_t tempF = sensor.GetTempF();
        uint32_t timeEnd = PAL.Millis();
        uint32_t timeDiff = timeEnd - timeNow;

        Log("Duration: ", timeDiff, " ms, (temp: ", tempF, ")");
    });

    // ok, so a wake, measure, sleep doesn't work
    // you have to wait for the duration associated with the resolution
    // 0.5   C =  30ms
    // 0.25  C =  65ms
    // 0.125 C = 130ms
    // 0.0625C = 250ms
    // so certainly 300ms is a reasonable threshold
    console.RegisterCommand("ttempwake", [](char *){
        Log(P("Timing a temperature reading; sleep -> wake -> measure -> sleep"));
        
        uint32_t timeNow = PAL.Millis();
        sensor.Wake();
        PAL.Delay(500);
        int8_t tempF = sensor.GetTempF();
        sensor.Sleep();
        uint32_t timeEnd = PAL.Millis();
        uint32_t timeDiff = timeEnd - timeNow;

        Log("Duration: ", timeDiff, " ms, (temp: ", tempF, ")");
    });
    
    console.RegisterCommand("ttempwd", [](char *){
        Log(P("Timing a temperature reading under watchdog; sleep -> wake -> measure -> sleep"));

        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_1000_MS);
        
        uint32_t timeNow = PAL.Millis();
        sensor.Wake();
        PAL.Delay(500);
        int8_t tempF = sensor.GetTempF();
        sensor.Sleep();
        uint32_t timeEnd = PAL.Millis();
        uint32_t timeDiff = timeEnd - timeNow;

        PAL.WatchdogDisable();

        Log("Duration: ", timeDiff, " ms, (temp: ", tempF, ")");
    });

    

    console.Start();
    evm.MainLoop();
}

void loop() {}
