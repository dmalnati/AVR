#ifndef __APP_PROTOTYPE_RADIO_BOX_1_H__
#define __APP_PROTOTYPE_RADIO_BOX_1_H__


#include <Evm.h>
#include <RFLink.h>
#include <LedFader.h>


struct AppPrototypeRadioBox1Config
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


class AppPrototypeRadioBox1
{
public:
    AppPrototypeRadioBox1(AppPrototypeRadioBox1Config &cfg);
    ~AppPrototypeRadioBox1();

    void Run() { Evm::GetInstance().MainLoop(); }

private:

    // Visual Interface System
    void StartupLightShow();
    void ShowRadioAddressRxTx();
    
    // Button Interface System
    void OnAttentionButton(uint8_t logicLevel);
    void OnFreeToTalkButton(uint8_t logicLevel);
    void OnYesButton(uint8_t logicLevel);
    void OnNoButton(uint8_t logicLevel);
    void OnClearButton(uint8_t logicLevel);
    
    // Application Messaging System
    enum class MessageType : uint8_t {
        MSG_ATTENTION,
        MSG_FREE_TO_TALK,
        MSG_YES,
        MSG_NO
    };

    struct Message
    {
        MessageType msgType;
    };

    void CreateAndSendMessageByType(MessageType msgType);
    void SendMessage(Message &msg);
    void OnMessageReceived(Message &msg);
    
    // Radio Control System
    void StartRadioSystem();
    void OnRxTxAddressChange(uint8_t logicLevel);
    void ReadRadioAddressRxTx();
    void OnRadioRXAvailable(uint8_t  srcAddr,
                            uint8_t  protocolId,
                            uint8_t *buf,
                            uint8_t  bufSize);
    void RadioTX(uint8_t *buf, uint8_t bufSize);
    void OnRadioTXComplete();
    
    // External Event Monitoring
    void StartButtonMonitoring();
    void StartRxTxAddressMonitoring();
    
    
    // State keeping
    AppPrototypeRadioBox1Config   &cfg_;
    RFLink<AppPrototypeRadioBox1> *rfLink_;
    uint8_t                        radioAddressRx_;
    uint8_t                        radioAddressTx_;
    
    // LED Control
    LEDFader                       ledFader_;
};



#endif  // __APP_PROTOTYPE_RADIO_BOX_1_H__



























