#include "Evm.h"
#include "Log.h"
#include "SensorCurrentVoltageINA3221.h"


static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;
static SensorCurrentVoltageINA3221 sensor;
static SensorCurrentVoltageINA3221::Channel *c1 = sensor.GetChannel1();
static SensorCurrentVoltageINA3221::Channel *c2 = sensor.GetChannel2();
static SensorCurrentVoltageINA3221::Channel *c3 = sensor.GetChannel3();


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

        // Read channel 1
        retVal = c1->GetShuntVoltage(regVal);
        Log("C1:GetShuntVoltage(", retVal, "): ", LogBIN(regVal), ", ", regVal);
        retVal = c1->GetBusVoltage(regVal);
        Log("C1:GetBusVoltage(", retVal, ")  : ", LogBIN(regVal), ", ", regVal);

        // Read channel 2
        retVal = c2->GetShuntVoltage(regVal);
        Log("C2:GetShuntVoltage(", retVal, "): ", LogBIN(regVal), ", ", regVal);
        retVal = c2->GetBusVoltage(regVal);
        Log("C2:GetBusVoltage(", retVal, ")  : ", LogBIN(regVal), ", ", regVal);

        // Read channel 3
        retVal = c3->GetShuntVoltage(regVal);
        Log("C3:GetShuntVoltage(", retVal, "): ", LogBIN(regVal), ", ", regVal);
        retVal = c3->GetBusVoltage(regVal);
        Log("C3:GetBusVoltage(", retVal, ")  : ", LogBIN(regVal), ", ", regVal);

        

        LogNL();
    });
    ted.RegisterForTimedEventInterval(1000);

    evm.MainLoop();
}

void loop() {}







