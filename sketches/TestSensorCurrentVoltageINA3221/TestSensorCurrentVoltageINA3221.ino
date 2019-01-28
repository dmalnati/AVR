#include "Evm.h"
#include "Log.h"
#include "SensorCurrentVoltageINA3221.h"


static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;
static SensorCurrentVoltageINA3221 sensor;
static SensorCurrentVoltageINA3221::Channel *c1 = sensor.GetChannel1();
static SensorCurrentVoltageINA3221::Channel *c2 = sensor.GetChannel2();
static SensorCurrentVoltageINA3221::Channel *c3 = sensor.GetChannel3();


void DoC1VoltageReadSpeedTest()
{
    uint16_t milliAmps = 0;
    uint32_t countGood = 0;
    uint32_t countBad  = 0;
    uint32_t timeStart = PAL.Micros();
    while (PAL.Micros() - timeStart < 1000000)
    {
        if (c1->GetShuntMilliAmps(milliAmps))
        {
            ++countGood;
        }
        else
        {
            ++countBad;
        }
    }

    Log("Timed good (", countGood, "/sec) and bad (", countBad, "/sec)");
}

void setup()
{
    LogStart(9600);
    Log("Starting");

    ted.SetCallback([](){
        Log("Sensing");

        uint8_t  retVal;
        uint16_t regVal = 0;

        // Get configuration
        retVal = sensor.GetRegConfiguration(regVal);
        Log("GetRegCfg(", retVal, "): ", LogBIN(regVal));

        sensor.PrintConfiguration();
        LogNL();

        retVal = sensor.SetChannel2Enable(0);
        Log("CH2 Enable(0) = ", retVal);
        retVal = sensor.SetAverageSampleCount(16);
        Log("SetAverageSampleCount(16) = ", retVal);
        retVal = sensor.SetBusVoltageConversionTimeUs(204);
        Log("GetBusVoltageConversionTimeUs(204) = ", retVal);
        retVal = sensor.SetShuntVoltageConversionTimeUs(204);
        Log("GetShuntVoltageConversionTimeUs(204) = ", retVal);
        retVal = sensor.SetOperatingMode(0b00000101);   // shunt voltage, continuous
        Log("SetOperatingMode(0b00000101) = ", retVal);
        retVal = sensor.GetRegConfiguration(regVal);
        Log("GetRegCfg(", retVal, "): ", LogBIN(regVal));
        sensor.PrintConfiguration();
        LogNL();

        // Speed Test
        DoC1VoltageReadSpeedTest();
        

        retVal = sensor.SetChannel2Enable(1);
        Log("CH2 Enable(1) = ", retVal);
        retVal = sensor.SetAverageSampleCount(1);
        Log("SetAverageSampleCount(1) = ", retVal);
        retVal = sensor.SetBusVoltageConversionTimeUs(1100);
        Log("GetBusVoltageConversionTimeUs(1100) = ", retVal);
        retVal = sensor.SetShuntVoltageConversionTimeUs(1100);
        Log("GetShuntVoltageConversionTimeUs(1100) = ", retVal);
        retVal = sensor.SetOperatingMode(0b00000111);   // shunt,bus voltage, continuous
        Log("SetOperatingMode(0b00000111) = ", retVal);
        retVal = sensor.GetRegConfiguration(regVal);
        Log("GetRegCfg(", retVal, "): ", LogBIN(regVal));
        sensor.PrintConfiguration();
        LogNL();

        

        uint16_t microVolts = 0;
        uint16_t milliAmps  = 0;
        uint16_t milliVolts = 0;

        // Read channel 1
        retVal = c1->GetShuntMicroVolts(microVolts);
        Log("C1:GetShuntMicroVolts(", retVal, "): ", microVolts);
        retVal = c1->GetShuntMilliAmps(milliAmps);
        Log("C1:GetShuntMilliAmps(", retVal, "): ", milliAmps);
        retVal = c1->GetBusMilliVolts(milliVolts);
        Log("C1:GetBusMilliVolts(", retVal, "): ", milliVolts);
        
        // Read channel 2
        retVal = c2->GetShuntMicroVolts(microVolts);
        Log("C2:GetShuntMicroVolts(", retVal, "): ", microVolts);
        retVal = c2->GetShuntMilliAmps(milliAmps);
        Log("C2:GetShuntMilliAmps(", retVal, "): ", milliAmps);
        retVal = c2->GetBusMilliVolts(milliVolts);
        Log("C2:GetBusMilliVolts(", retVal, "): ", milliVolts);
        
        // Read channel 3
        retVal = c3->GetShuntMicroVolts(microVolts);
        Log("C3:GetShuntMicroVolts(", retVal, "): ", microVolts);
        retVal = c3->GetShuntMilliAmps(milliAmps);
        Log("C3:GetShuntMilliAmps(", retVal, "): ", milliAmps);
        retVal = c3->GetBusMilliVolts(milliVolts);
        Log("C3:GetBusMilliVolts(", retVal, "): ", milliVolts);


        // Speed test
        DoC1VoltageReadSpeedTest();


        LogNL();
        LogNL();
        LogNL();
        LogNL();
        LogNL();
    });
    ted.RegisterForTimedEventInterval(1000);

    evm.MainLoop();
}

void loop() {}







