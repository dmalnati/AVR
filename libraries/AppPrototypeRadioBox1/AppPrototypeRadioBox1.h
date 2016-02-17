#ifndef __APP_PROTOTYPE_RADIO_BOX_1_H__
#define __APP_PROTOTYPE_RADIO_BOX_1_H__


#include <Evm.h>
#include <RFLink.h>



struct AppPrototypeRadioBox1Config
{
    uint8_t pinDipAddressRx1;
    uint8_t pinDipAddressRx2;
    uint8_t pinDipAddressTx1;
    uint8_t pinDipAddressTx2;
    
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
    
    uint8_t valRadioRealm;
    uint8_t valProtocolId;
    uint8_t pinRadioRX;
    uint8_t pinRadioTX;
};



class AppPrototypeRadioBox1
{
public:
    AppPrototypeRadioBox1(AppPrototypeRadioBox1Config &cfg);
    ~AppPrototypeRadioBox1();

    void Run() { Evm::GetInstance().MainLoop(); }

private:
    void StartupLightShow();
    
    
    
    void StartRadioSystem();
    void OnRadioRXAvailable(uint8_t  srcAddr,
                            uint8_t  protocolId,
                            uint8_t *buf,
                            uint8_t  bufSize);
    void OnRadioTXComplete();
    
    void ReadRadioAddressRxTx();
    void ShowRadioAddressRxTx();
    void StartRxTxAddressMonitoring();
    void OnDipAddressRx1(uint8_t logicLevel);
    void OnDipAddressRx2(uint8_t logicLevel);
    void OnDipAddressTx1(uint8_t logicLevel);
    void OnDipAddressTx2(uint8_t logicLevel);

    
    void StartButtonMonitoring();
    void OnAttentionButton(uint8_t logicLevel);
    void OnFreeToTalkButton(uint8_t logicLevel);
    void OnYesButton(uint8_t logicLevel);
    void OnNoButton(uint8_t logicLevel);
    void OnClearButton(uint8_t logicLevel);
    
    
    
    
    
    AppPrototypeRadioBox1Config &cfg_;
    
    RFLink<AppPrototypeRadioBox1> *rfLink_;
    uint8_t                        radioAddressRx_;
    uint8_t                        radioAddressTx_;
};



#endif  // __APP_PROTOTYPE_RADIO_BOX_1_H__



























