#ifndef __APP_SIMPLE_IPC_ON_SERIAL_LINK_CONTROL_H__
#define __APP_SIMPLE_IPC_ON_SERIAL_LINK_CONTROL_H__


#include "Evm.h"
#include "CommsSimpleIPCOnSerialLink.h"
#include "PinState.h"
#include "PinStateSimpleIPCController.h"

class AppSimpleIPCOnSerialLinkControl
{
public:

    void Run()
    {
        // Associate controllers
        pinStateController_.Init(&pinState_);
        
        // Register controllers
        comms_.RegisterProtocolMessageHandler(&pinStateController_);
        
        // Init comms
        comms_.Init();
        
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

    PinState                    pinState_;
    PinStateSimpleIPCController pinStateController_;

    CommsSimpleIPCOnSerialLink<10,1,10> comms_;
};


#endif  // __APP_SIMPLE_IPC_ON_SERIAL_LINK_CONTROL_H__