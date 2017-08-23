#ifndef __APP_TEST_SENSOR_TEMPERATURE_DHTXX_SIMPLE_IPC_CONTROLLER_H__
#define __APP_TEST_SENSOR_TEMPERATURE_DHTXX_SIMPLE_IPC_CONTROLLER_H__


#include "Evm.h"
#include "CommsSimpleIPCOnSerialAndRFLink.h"
#include "SensorTemperatureDHTXX.h"
#include "SensorTemperatureDHTXXSimpleIPCController.h"


struct AppTestSensorTemperatureDHTXXSimpleIPCControllerConfig
{
    uint8_t realm;
    uint8_t srcAddr;
    uint8_t dstAddr;
    
    uint8_t pinRFRX;
    uint8_t pinRFTX;
    
    uint8_t pinSensor;
};


class AppTestSensorTemperatureDHTXXSimpleIPCController
{
public:

    AppTestSensorTemperatureDHTXXSimpleIPCController(AppTestSensorTemperatureDHTXXSimpleIPCControllerConfig &cfg)
    : cfg_(cfg)
    , s_(cfg_.pinSensor)
    {
        // Nothing to do
    }

    void Run()
    {
        // Init sensor
        s_.Init();
        
        // Associate controllers
        sc_.Init(&s_);
        
        // Register controllers
        comms_.RegisterProtocolMessageHandler(&sc_);
        
        // Init comms
        comms_.Init(cfg_.realm,
                    cfg_.srcAddr,
                    cfg_.dstAddr,
                    cfg_.pinRFRX,
                    cfg_.pinRFTX);
        
        // Handle events
        evm_.MainLoop();
    }

    
private:
    // Calculate sizing values for Evm
    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;    
private:

    AppTestSensorTemperatureDHTXXSimpleIPCControllerConfig &cfg_;

    SensorTemperatureDHTXX  s_;
    SensorTemperatureDHTXXSimpleIPCController sc_;

    CommsSimpleIPCOnSerialAndRFLink<10,1,10> comms_;
};


#endif  // __APP_TEST_SENSOR_TEMPERATURE_DHTXX_SIMPLE_IPC_CONTROLLER_H__