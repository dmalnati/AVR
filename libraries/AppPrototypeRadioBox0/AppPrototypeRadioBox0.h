#ifndef __APP_PROTOTYPE_RADIO_BOX_0_H__
#define __APP_PROTOTYPE_RADIO_BOX_0_H__


//#include <RFLink.h>
//#include <LedFader.h>


struct AppPrototypeRadioBox0Config
{
    // Visual and Button Interface System
    uint8_t pinAttentionButton;
    uint8_t pinAttentionRedLED;
    uint8_t pinAttentionGreenLED;
    uint8_t pinAttentionBlueLED;
    
    uint8_t pinFreeToTalkButton;
    uint8_t pinFreeToTalkLED;
    
    uint8_t pinYesButton;
    uint8_t pinYesLED;
    
    uint8_t pinNoButton;
    uint8_t pinNoLED;
    
    uint8_t pinClearButton;
    
    // Application Messaging System
    uint8_t valProtocolId;
    
    // Radio Link Layer
    uint8_t valRealm;
    uint8_t pinDipAddressRx1;
    uint8_t pinDipAddressRx2;
    uint8_t pinDipAddressTx1;
    uint8_t pinDipAddressTx2;
    
    // Radio Physical Layer
    uint8_t  pinRadioRX;
    uint8_t  pinRadioTX;
    uint16_t valBaud;
};


class AppPrototypeRadioBox0
{
public:
    AppPrototypeRadioBox0(AppPrototypeRadioBox0Config &cfg);
    ~AppPrototypeRadioBox0();

    void Run();

private:

    // Visual Interface System
    void StartupLightShow();
    void ShowRadioAddressRxTx();
    
    
    // Radio Control System
    void ReadRadioAddressRxTx();
    
    // External Event Monitoring
    void StartRxTxAddressMonitoring();
    
    
    // State keeping
    AppPrototypeRadioBox0Config   &cfg_;
    uint8_t                        radioAddressRx_;
    uint8_t                        radioAddressTx_;
};



#endif  // __APP_PROTOTYPE_RADIO_BOX_0_H__



























