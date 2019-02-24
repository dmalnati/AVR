#ifndef __APP_CURRENT_VOLTAGE_MON_H__
#define __APP_CURRENT_VOLTAGE_MON_H__


#include "Log.h"
#include "Evm.h"
#include "SerialInput.h"
#include "TimedEventHandler.h"
#include "SensorCurrentVoltageINA3221.h"


struct AppCurrentVoltageMonConfig
{
    uint32_t baud;
};


class AppCurrentVoltageMon
{
    static const uint32_t DEFAULT_INTERVAL_MS = 1000;
    
public:
    AppCurrentVoltageMon(AppCurrentVoltageMonConfig &cfg)
    : cfg_(cfg)
    , intervalMs_(DEFAULT_INTERVAL_MS)
    , suvEnabled_(0)
    , c1_(sensor_.GetChannel1())
    , c2_(sensor_.GetChannel2())
    , c3_(sensor_.GetChannel3())
    {
        // Nothing to do
    }
    
    void Run()
    {
        LogStart(cfg_.baud);
        
        // Set up faster I2C comms
        SetupI2C();
        
        // Set up interactive commands
        SetupShell();
        
        // Handle events
        evm_.MainLoop();
    }
    
    
private:
    
    void SetupI2C()
    {
        I2C.SetFreqMax();
    }
    
    void SetupShell()
    {
        shell_.RegisterCommand("help", [this](char *) {
            PrintHelp();
        });

        shell_.RegisterCommand("iv", [this](char *cmdStr) {
            Str str(cmdStr);

            if (str.TokenCount(' ') >= 2)
            {
                const char *valStr = str.TokenAtIdx(1, ' ');
                uint32_t    val    = atol(valStr);
                
                intervalMs_ = val;
                
                Log(P("interval = "), intervalMs_, P(" ms"));
                LogNL();
                
                if (ted_.IsRegistered())
                {
                    shell_.Exec("start");
                }
            }
        });

        shell_.RegisterCommand("start", [this](char *) {
            LogNL(2);
            Log(P("[start: interval = "), intervalMs_, P(" ms, suv = "), suvEnabled_, ']');
            LogNL();
            PrintHeader();
            
            ted_.SetCallback([this](){ PrintMeasurements(); });
            ted_.RegisterForTimedEventIntervalRigid(intervalMs_, 0);
            timeAtStart_ = ted_.GetTimeQueued();
        });
        
        shell_.RegisterCommand("stop", [this](char *) {
            ted_.DeRegisterForTimedEvent();
            
            LogNL(2);
            Log(P("[stop]"));
            LogNL();
        });
        
        shell_.RegisterCommand("suv", [this](char *) {
            suvEnabled_ = !suvEnabled_;
            
            Log(P("suv = "), suvEnabled_);
            LogNL();
            
            if (ted_.IsRegistered())
            {
                shell_.Exec("start");
            }
        });
        
        shell_.SetVerbose(0);
        shell_.Start();
        
        PrintHelp();
    }
    
    void PrintHelp()
    {
        LogNL(5);
        Log(P("[3-Channel Current/Voltage Monitor]"));
        LogNL();
        Log(P("Commands"));
        Log(P("--------"));
        Log(P("help  - print this"));
        Log(P("iv    - reporting interval in ms"));
        Log(P("start - begin reporting"));
        Log(P("stop  - stop reporting"));
        Log(P("suv   - toggle showing shunt voltage"));
        LogNL();
        Log(P("Output Table Values"));
        Log(P("------"));
        Log(P("Chan.mV  - Voltage, in milli-volts"));
        Log(P("Chan.mA  - Current, in milli-amps"));
        Log(P("Chan.suV - Shunt voltage, in micro-amps (optional)"));
        LogNL();
        Log(P("Status"));
        Log(P("------"));
        Log(P("interval = "), intervalMs_, P(" ms"));
        Log(P("suv      = "), suvEnabled_);
        LogNL();
    }
    
    void PrintHeader()
    {
        LogNNL("    ms");
        
        for (uint8_t i = 0; i < 3; ++i)
        {
            LogNNL(",  C", i + 1, ".mV");  // GetBusMilliVolts
            LogNNL(",  C", i + 1, ".mA");  // GetShuntMilliAmps
            
            if (suvEnabled_)
            {
                LogNNL(", C", i + 1, ".suV"); // GetShuntMicroVolts
            }
        }
        
        LogNL();
    }
    
    void PrintMeasurements()
    {
        // Gather values without logging in between to improve closeness in time
        // of measurements.
        uint32_t timeNow = PAL.Millis();
        
        struct {
            int16_t mV;
            int16_t mA;
            int16_t suV;
        } chanReadingList[3];
        uint8_t idx = 0;
        for (auto c : (SensorCurrentVoltageINA3221::Channel*[]){ c1_, c2_, c3_ })
        {
            c->GetBusMilliVolts(chanReadingList[idx].mV);
            c->GetShuntMilliAmps(chanReadingList[idx].mA);
            c->GetShuntMicroVolts(chanReadingList[idx].suV);
            
            idx += 1;
        }
        
        // Log captured values
        uint32_t timeSinceStart = timeNow - timeAtStart_;
        
        printf("%6lu", timeSinceStart);
        
        for (auto &chanReading : chanReadingList)
        {
            printf(", %6i", chanReading.mV);
            printf(", %6i", chanReading.mA);
            
            if (suvEnabled_)
            {
                printf(", %6i", chanReading.suV);
            }
        }
        
        LogNL();
    }

    
private:

    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
    AppCurrentVoltageMonConfig &cfg_;
    
    uint32_t intervalMs_;
    uint8_t  suvEnabled_;
    
    SensorCurrentVoltageINA3221           sensor_;
    SensorCurrentVoltageINA3221::Channel *c1_;
    SensorCurrentVoltageINA3221::Channel *c2_;
    SensorCurrentVoltageINA3221::Channel *c3_;
    
    TimedEventHandlerDelegate ted_;
    uint32_t timeAtStart_;
    
    static const uint8_t NUM_COMMANDS = 16;
    SerialAsyncConsoleEnhanced<NUM_COMMANDS>  shell_;
};


#endif  // __APP_CURRENT_VOLTAGE_MON_H__










