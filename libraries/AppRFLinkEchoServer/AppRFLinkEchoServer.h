#ifndef __APP_RF_LINK_ECHO_SERVER_H__
#define __APP_RF_LINK_ECHO_SERVER_H__


#include "Evm.h"
#include "RFLink.h"
#include "LedFader.h"


struct AppRFLinkEchoServerConfig
{
    int8_t pinRFRX;
    int8_t pinRFTX;
    
    uint8_t pinLEDRX;
    uint8_t pinLEDTX;
    
    uint8_t realm;
    uint8_t srcAddr;
};

class AppRFLinkEchoServer
{
    using ThisClass     = AppRFLinkEchoServer;
    using RFLinkClass   = RFLink<AppRFLinkEchoServer>;
    using LEDFaderClass = LEDFader<1,1>;
    
public:
    AppRFLinkEchoServer(AppRFLinkEchoServerConfig &cfg)
    : cfg_(cfg)
    {
        // Nothing to do
    }
    
    ~AppRFLinkEchoServer() { }
    
    void Run()
    {
        // Init link
        rfLink_.Init(this,
                     cfg_.pinRFRX,
                     &ThisClass::OnRxAvailable,
                     cfg_.pinRFTX);
                     
        rfLink_.SetSrcAddr(cfg_.srcAddr);
        
        // Receive all messages, regardless of whether they're addressed to me
        // or not.
        rfLink_.EnablePromiscuousMode();
        
        // Set up faders
        ledFaderRX_.AddLED(cfg_.pinLEDRX);
        ledFaderTX_.AddLED(cfg_.pinLEDTX);
        
        // Flash to show power on
        ledFaderRX_.FadeOnce();
        ledFaderTX_.FadeOnce();
        
        // Handle events
        evm_.MainLoop();
    }
    
    void OnRxAvailable(RFLinkHeader *hdr, uint8_t *buf, uint8_t bufSize)
    {
        // Promiscuous mode is enabled, so do some filtering.
        // Only reply back if the message was for you, but indicate that the
        // message was received regardless.
        
        ledFaderRX_.FadeOnce();
        
        if (hdr->realm == cfg_.realm && hdr->dstAddr == cfg_.srcAddr)
        {
            // Echo
            rfLink_.SendTo(hdr->srcAddr, hdr->protocolId, buf, bufSize);
            
            ledFaderTX_.FadeOnce();
        }
    }

private:

    // Calculate sizing values for Evm
    static const uint8_t C_IDLE  =
        RFLinkClass::C_IDLE +
        LEDFaderClass::C_IDLE * 2;
    static const uint8_t C_TIMED =
        RFLinkClass::C_TIMED +
        LEDFaderClass::C_TIMED * 2;
    static const uint8_t C_INTER =
        RFLinkClass::C_INTER +
        LEDFaderClass::C_IDLE * 2;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
private:

    AppRFLinkEchoServerConfig &cfg_;

    RFLinkClass rfLink_;
    
    LEDFaderClass ledFaderRX_;
    LEDFaderClass ledFaderTX_;
};


#endif  // __APP_RF_LINK_ECHO_SERVER_H__


