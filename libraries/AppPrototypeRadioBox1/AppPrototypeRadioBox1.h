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

    void Run();

private:

    // Calculate sizing values for Evm
    static const uint8_t C_IDLE  = 4;
    static const uint8_t C_TIMED = 1;
    static const uint8_t C_INTER = 3;

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

private:
    const uint32_t DEFAULT_FADER_DURATION_MS            = 600;
    const uint32_t DEFAULT_DELAY_BEFORE_ADDR_DISPLAY_MS = 150;


    // Visual Interface System
    void StartupLightShow();
    void ShowLedFadeStartupSequence();
    void ShowRadioAddressRxTx();
    
    enum class StartupFadeDirection : uint8_t {
        LeftToRight,
        RightToLeft
    };
    void AnimateFadeLeftToRight(uint32_t durationMs);
    void AnimateFadeRightToLeft(uint32_t durationMs);
    void AnimateFadeSweep(uint32_t             durationMs,
                          StartupFadeDirection fadeDirection);
    
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
    void StartAllMonitoring();
    void StopAllMonitoring();
    void StartButtonMonitoring();
    void StopButtonMonitoring();
    void StartRxTxAddressMonitoring();
    void StopRxTxAddressMonitoring();
    
    
    
    // Configuration
    AppPrototypeRadioBox1Config &cfg_;
    
    // Radio Control
    RFLink<AppPrototypeRadioBox1> *rfLink_;
    uint8_t                        radioAddressRx_;
    uint8_t                        radioAddressTx_;
    
    // LED Control
    LEDFader<2,1>  ledFaderStartup1_;
    LEDFader<2,1>  ledFaderStartup2_;
    LEDFader<2,1>  ledFaderStartup3_;
    
    LEDFader<4,1>  ledFaderRadioAddress_;
    
    LEDFader<1,1>  ledFaderAttentionRedLED_;
    LEDFader<1,1>  ledFaderAttentionGreenLED_;
    LEDFader<1,1>  ledFaderAttentionBlueLED_;
    LEDFader<1,1>  ledFaderFreeToTalkLED_;
    LEDFader<1,1>  ledFaderYesLED_;
    LEDFader<1,1>  ledFaderNoLED_;
    
    // Interrupt monitoring
    using PinToFn = InterruptEventHandlerDelegate<AppPrototypeRadioBox1>;

    PinToFn ptfAttention_;
    PinToFn ptfFreeToTalk_;
    PinToFn ptfYes_;
    PinToFn ptfNo_;
    PinToFn ptfClear_;
    
    PinToFn ptfDipAddressRx1_;
    PinToFn ptfDipAddressRx2_;
    PinToFn ptfDipAddressTx1_;
    PinToFn ptfDipAddressTx2_;
    
};



#endif  // __APP_PROTOTYPE_RADIO_BOX_1_H__



























