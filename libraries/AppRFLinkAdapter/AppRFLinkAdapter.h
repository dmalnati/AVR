#ifndef __APP_RF_LINK_ADAPTER__
#define __APP_RF_LINK_ADAPTER__


#include "Evm.h"
#include "SerialLink.h"
#include "RFLink.h"
#include "LedFader.h"


struct AppRFLinkAdapterConfig
{
    uint8_t pinSerialRX;
    uint8_t pinSerialTX;
    uint8_t serialProtocolId;
    
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
        // Set up SerialLink
        serialLink_.Init(this, &ThisClass::OnSerialRxAvailable);
        
        // Set up RFLink
        rfLinkRaw_.Init(this,
                        cfg_.pinRFRX,
                        &ThisClass::OnRFRxAvailable,
                        cfg_.pinRFTX,
                        &ThisClass::OnRFTxComplete);
                     
        // Set up LedFaders
        ledFaderRx_.AddLED(cfg_.pinLEDRX);
        ledFaderTx_.AddLED(cfg_.pinLEDTX);
        
        // Handle events
        evm_.MainLoop();
    }

private:
    // Calculate sizing values for Evm
    static const uint8_t C_IDLE  = 
        SerialLink<ThisClass>::C_IDLE +
        RFLink_Raw<ThisClass>::C_IDLE +
        LEDFader<1,1>::C_IDLE * 2;
    static const uint8_t C_TIMED = 
        SerialLink<ThisClass>::C_TIMED +
        RFLink_Raw<ThisClass>::C_TIMED +
        LEDFader<1,1>::C_TIMED * 2;
    static const uint8_t C_INTER = 
        SerialLink<ThisClass>::C_INTER +
        RFLink_Raw<ThisClass>::C_INTER +
        LEDFader<1,1>::C_INTER * 2;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

private:

    void OnSerialRxAvailable(SerialLink<ThisClass>::Header */*hdr*/,
                             uint8_t                       *buf,
                             uint8_t                        bufSize)
    {
        rfLinkRaw_.Send(buf, bufSize);
    }
    
    void OnRFRxAvailable(uint8_t *buf, uint8_t bufSize)
    {
        serialLink_.Send(cfg_.serialProtocolId, buf, bufSize);
    }
    
    void OnRFTxComplete()
    {
        // Nothing to do
    }

    AppRFLinkAdapterConfig &cfg_;
    
    SerialLink<ThisClass> serialLink_;
    RFLink_Raw<ThisClass> rfLinkRaw_;
    LEDFader<1,1>         ledFaderRx_;
    LEDFader<1,1>         ledFaderTx_;
};


#endif  // __APP_RF_LINK_ADAPTER__










