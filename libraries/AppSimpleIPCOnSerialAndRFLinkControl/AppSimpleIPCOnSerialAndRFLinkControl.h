#ifndef __APP_SIMPLE_IPC_ON_SERIAL_AND_RF_LINK_CONTROL_H__
#define __APP_SIMPLE_IPC_ON_SERIAL_AND_RF_LINK_CONTROL_H__


#include "Evm.h"
#include "CommsSimpleIPCOnSerialAndRFLink.h"
#include "PinState.h"
#include "PinStateSimpleIPCController.h"


struct AppSimpleIPCOnSerialAndRFLinkControlConfig
{
    uint8_t realm;
    uint8_t srcAddr;
    uint8_t dstAddr;
    
    uint8_t pinRFRX;
    uint8_t pinRFTX;
};


class AppSimpleIPCOnSerialAndRFLinkControl
{
public:

    AppSimpleIPCOnSerialAndRFLinkControl(AppSimpleIPCOnSerialAndRFLinkControlConfig &cfg)
    : cfg_(cfg)
    {
        // Nothing to do
    }

    void Run()
    {
        // Associate controllers
        pinStateController_.Init(&pinState_);
        
        // Register controllers
        comms_.RegisterProtocolMessageHandler(&pinStateController_);
        
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

    AppSimpleIPCOnSerialAndRFLinkControlConfig &cfg_;

    PinState                    pinState_;
    PinStateSimpleIPCController pinStateController_;

    CommsSimpleIPCOnSerialAndRFLink<10,1,10> comms_;
};


#endif  // __APP_SIMPLE_IPC_ON_SERIAL_AND_RF_LINK_CONTROL_H__