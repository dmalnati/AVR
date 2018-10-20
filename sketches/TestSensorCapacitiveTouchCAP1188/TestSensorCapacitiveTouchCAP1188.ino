#include "Evm.h"
#include "TimedEventHandler.h"
#include "SerialInput.h"
#include "SensorCapacitiveTouchCAP1188.h"


static Evm::Instance<20,20,20> evm;
static SerialAsyncConsoleEnhanced<30> shell;

static const uint8_t CAP_I2C_ADDR = 0x28;
static SensorCapacitiveTouchCAP1188 cap(CAP_I2C_ADDR);

static TimedEventHandlerDelegate ted;


void setup()
{
    LogStart(9600);
    Log(P("Starting"));

    // Set up touch sensor


    


    // Set up input
    shell.RegisterCommand("sens", [](char *cmdStr){
      Str str(cmdStr);
    
      if (str.TokenCount(' ') == 2)
      {
          uint8_t multFactor = atoi(str.TokenAtIdx(1, ' '));

          Log("Mult Factor: ", multFactor);

          cap.SetSensitivity(multFactor);
      }
    });
    
    shell.RegisterCommand("ssens", [](char *cmdStr){
      Str str(cmdStr);
    
      if (str.TokenCount(' ') == 2)
      {
          uint8_t multFactor = atoi(str.TokenAtIdx(1, ' '));

          Log("Standby Mult Factor: ", multFactor);

          cap.SetStandbySensitivity(multFactor);
      }
    });

    shell.RegisterCommand("enable", [](char *cmdStr){
      Str str(cmdStr);
    
      if (str.TokenCount(' ') == 2)
      {
          uint8_t bitmap = atoi(str.TokenAtIdx(1, ' '));

          Log("Enable Bitmap: ", LogBIN(bitmap));

          cap.SetInputEnable(bitmap);
      }
    });

    shell.RegisterCommand("senable", [](char *cmdStr){
      Str str(cmdStr);
    
      if (str.TokenCount(' ') == 2)
      {
          uint8_t bitmap = atoi(str.TokenAtIdx(1, ' '));

          Log("Standby Enable Bitmap: ", LogBIN(bitmap));

          cap.SetStandbyInputEnable(bitmap);
      }
    });

    

    
    shell.RegisterCommand("get", [](char *){
        Log("Get");

        uint8_t bitmapTouched = cap.GetTouched();
        
        Log("Got: ", LogBIN(bitmapTouched));
    });

    shell.RegisterCommand("start", [](char *){
        Log("Start");

        cap.SetCallback([](uint8_t bitmapTouched){
            Log("Touched: ", LogBIN(bitmapTouched));
        });
        cap.Start();
    });

    shell.RegisterCommand("stop", [](char *){
        Log("Stop");

        cap.Stop();
    });






    shell.RegisterCommand("pactive", [](char *){
        Log("Power: Active");

        cap.PowerActive();
    });

    shell.RegisterCommand("pstandby", [](char *){
        Log("Power: Standby");

        cap.PowerStandby();
    });



/*
# SUM, 1 sample, 1280us sample time, 140ms cycle
# 9-26us, 0.5 inch range
sconf 1 0 2 3

# AVG, 1 sample, 1280us sample time, 140ms cycle
# 9-26us, 0.5 inch range
sconf 0 7 2 3

# AVG, 1 sample, 2650us sample time, 140ms cycle
# 15-32us, 1.0 inch range
sconf 0 0 3 3

# AVG, 1 sample, 320us sample time, 140ms cycle
# 9-17us, 0 inch range
# (change to SUM didn't help)
sconf 0 0 0 3
*/
    shell.RegisterCommand("sconf", [](char *cmdStr){
        Str str(cmdStr);
        
        Log("Standby Configure");

        if (str.TokenCount(' ') == 5)
        {
            SensorCapacitiveTouchCAP1188::StandbyAvgOrSum               avgOrSum              = (SensorCapacitiveTouchCAP1188::StandbyAvgOrSum)atoi(str.TokenAtIdx(1, ' '));
            SensorCapacitiveTouchCAP1188::StandbySamplesPerMeasurement  samplesPerMeasurement = (SensorCapacitiveTouchCAP1188::StandbySamplesPerMeasurement)atoi(str.TokenAtIdx(2, ' '));
            SensorCapacitiveTouchCAP1188::StandbySampleTime             sampleTime            = (SensorCapacitiveTouchCAP1188::StandbySampleTime)atoi(str.TokenAtIdx(3, ' '));
            SensorCapacitiveTouchCAP1188::StandbyCycleTime              cycleTime             = (SensorCapacitiveTouchCAP1188::StandbyCycleTime)atoi(str.TokenAtIdx(4, ' '));
            
            cap.StandbyConfigure(avgOrSum, samplesPerMeasurement, sampleTime, cycleTime);
        }
    });


/*

Test routine

pstandby
sconf 0 0 3 3
senable 128
ssens 7
start

*/


    
    shell.Start();

    
    Log("Running");
    evm.MainLoop();
}



void loop(){}




