#ifndef __APP_RF_LINK_ADAPTER__
#define __APP_RF_LINK_ADAPTER__


#include "Evm.h"
#include "SerialLink.h"
#include "RFLink.h"


struct AppRFLinkAdapterConfig
{
    uint8_t pinSerialRX;
    uint8_t pinSerialTX;
    
    uint8_t pinRFRX;
    uint8_t pinRFTX;
    
    uint8_t pinLEDRX;
    uint8_t pinLEDTX;
};

class AppRFLinkAdapter
{
    using ThisClass = AppRFLinkAdapter;
    
public:
    AppRFLinkAdapter(AppRFLinkAdapterConfig &cfg)
    : cfg_(cfg)
    {
        // Nothing to do
    }
    
    ~AppRFLinkAdapter() { }
    
    void Run()
    {
        serialLink_.Init(this, &ThisClass::OnSerialRxAvailable);
        evm_.MainLoop();
    }

private:
    // Calculate sizing values for Evm
    static const uint8_t C_IDLE  = 0;
    static const uint8_t C_TIMED = 0;
    static const uint8_t C_INTER = 0;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

private:

    void OnSerialRxAvailable(SerialLink<ThisClass>::Header */*hdr*/,
                             uint8_t */*buf*/,
                             uint8_t /*len*/)
    {
        
    }

    AppRFLinkAdapterConfig &cfg_;
    
    SerialLink<ThisClass> serialLink_;
};


#endif  // __APP_RF_LINK_ADAPTER__










