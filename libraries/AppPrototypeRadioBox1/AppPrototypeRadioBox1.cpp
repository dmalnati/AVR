#include <util/atomic.h>

#include "Utl.h"
#include "AppPrototypeRadioBox1.h"







//////////////////////////////////////////////////////////////////////
//
// Constructor and Destructor
//
//////////////////////////////////////////////////////////////////////

AppPrototypeRadioBox1::
AppPrototypeRadioBox1(AppPrototypeRadioBox1Config &cfg)
: evm_()
, cfg_(cfg)
, rfLink_(NULL)
, radioAddressRx_(0)
, radioAddressTx_(0)
, ledFader_()
{
    // Nothing to do
}

AppPrototypeRadioBox1::
~AppPrototypeRadioBox1()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        delete rfLink_;
    }
}



//////////////////////////////////////////////////////////////////////
//
// Run
//
//////////////////////////////////////////////////////////////////////

void AppPrototypeRadioBox1::
Run()
{
    // Read in dynamic configuration
    ReadRadioAddressRxTx();
    
    // Dazzle
    StartupLightShow();
    
    // Enable Radio
    StartRadioSystem();
    
    // Start watching buttons
    StartButtonMonitoring();
    
    // Begin application
    evm_.MainLoop();
}


//////////////////////////////////////////////////////////////////////
//
// Visual Interface System
//
//////////////////////////////////////////////////////////////////////


void AppPrototypeRadioBox1::
StartupLightShow()
{
    ShowLedFadeStartupSequence();
    ShowRadioAddressRxTx();
}

void AppPrototypeRadioBox1::
ShowLedFadeStartupSequence()
{
    
    /*
    
    figure out min number of ledFaders you can use for every light sequence
    intended in application.
    
     */
    
    LEDFader<6,1> fader;
    
    fader.AddLED(cfg_.pinAttentionRedLED);
    fader.AddLED(cfg_.pinAttentionBlueLED);
    fader.AddLED(cfg_.pinAttentionGreenLED);
    fader.AddLED(cfg_.pinFreeToTalkLED);
    fader.AddLED(cfg_.pinYesLED);
    fader.AddLED(cfg_.pinNoLED);
    
    fader.FadeForever(500);
    if (!evm_.HoldStackDangerously(0, 3000))
    {
        PinToggle(cfg_.pinNoButton, 50);
        PAL.Delay(50);
        PinToggle(cfg_.pinNoButton, 50);
        PAL.Delay(50);
        PinToggle(cfg_.pinNoButton, 50);
        PAL.Delay(50);
        PinToggle(cfg_.pinNoButton, 50);
        PAL.Delay(50);
        PinToggle(cfg_.pinNoButton, 50);
    }
}


void AppPrototypeRadioBox1::
ShowRadioAddressRxTx()
{
    PAL.DigitalWrite(cfg_.pinAttentionBlueLED,
                     radioAddressRx_ & 0x02 ? HIGH : LOW);
    PAL.DigitalWrite(cfg_.pinFreeToTalkLED,
                     radioAddressRx_ & 0x01 ? HIGH : LOW);
    PAL.DigitalWrite(cfg_.pinYesLED, 
                     radioAddressTx_ & 0x02 ? HIGH : LOW);
    PAL.DigitalWrite(cfg_.pinNoLED,
                     radioAddressTx_ & 0x01 ? HIGH : LOW);
    
    PAL.DigitalWrite(cfg_.pinAttentionBlueLED, LOW);
    PAL.DigitalWrite(cfg_.pinFreeToTalkLED,    LOW);
    PAL.DigitalWrite(cfg_.pinYesLED,           LOW);
    PAL.DigitalWrite(cfg_.pinNoLED,            LOW);
}


//////////////////////////////////////////////////////////////////////
//
// Button Interface System
//
//////////////////////////////////////////////////////////////////////

void AppPrototypeRadioBox1::
OnAttentionButton(uint8_t)
{
    CreateAndSendMessageByType(MessageType::MSG_ATTENTION);
}


void AppPrototypeRadioBox1::
OnFreeToTalkButton(uint8_t)
{
    CreateAndSendMessageByType(MessageType::MSG_FREE_TO_TALK);
}

void AppPrototypeRadioBox1::
OnYesButton(uint8_t)
{
    CreateAndSendMessageByType(MessageType::MSG_YES);
}

void AppPrototypeRadioBox1::
OnNoButton(uint8_t)
{
    PinToggle(cfg_.pinYesLED);
}

void AppPrototypeRadioBox1::
OnClearButton(uint8_t)
{
    PinToggle(cfg_.pinNoLED);
}


//////////////////////////////////////////////  ////////////////////////
//
// Application Messaging System
//
//////////////////////////////////////////////////////////////////////

void AppPrototypeRadioBox1::
CreateAndSendMessageByType(MessageType /* msgType */)
{
    Message msg = { .msgType = MessageType::MSG_ATTENTION };
    
    SendMessage(msg);
}

void AppPrototypeRadioBox1::
SendMessage(Message &msg)
{
    RadioTX((uint8_t *)&msg, sizeof(msg));
}

void AppPrototypeRadioBox1::
OnMessageReceived(Message &msg)
{
    switch (msg.msgType)
    {
        case MessageType::MSG_ATTENTION:
        {
            break;
        }
        
        default: break;
    }
}


//////////////////////////////////////////////////////////////////////
//
// Radio Control System
//
//////////////////////////////////////////////////////////////////////

void AppPrototypeRadioBox1::
StartRadioSystem()
{
    // Monitor for changes to the RX or TX Address
    StartRxTxAddressMonitoring();
    
    // Start Radio Handler
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        rfLink_ =
            new RFLink<AppPrototypeRadioBox1>(
                cfg_.valRealm,
                radioAddressRx_,
                this,
                cfg_.pinRadioRX,
                &AppPrototypeRadioBox1::OnRadioRXAvailable,
                cfg_.pinRadioTX,
                &AppPrototypeRadioBox1::OnRadioTXComplete,
                cfg_.valBaud
            );
    }
}

void AppPrototypeRadioBox1::
OnRxTxAddressChange(uint8_t /* logicLevel */)
{
    ReadRadioAddressRxTx();
    ShowRadioAddressRxTx();
    
    // Keep the radio system aware of the local address
    if (rfLink_)
    {
        rfLink_->SetSrcAddr(radioAddressRx_);
    }
}

void AppPrototypeRadioBox1::
ReadRadioAddressRxTx()
{
    // Set up DIP pins for reading
    PAL.PinMode(cfg_.pinDipAddressRx1, INPUT);
    PAL.PinMode(cfg_.pinDipAddressRx2, INPUT);
    PAL.PinMode(cfg_.pinDipAddressTx1, INPUT);
    PAL.PinMode(cfg_.pinDipAddressTx2, INPUT);
    
    radioAddressRx_ = (PAL.DigitalRead(cfg_.pinDipAddressRx1) << 1)|
                      (PAL.DigitalRead(cfg_.pinDipAddressRx2) << 0);
                      
    radioAddressTx_ = (PAL.DigitalRead(cfg_.pinDipAddressTx1) << 1) |
                      (PAL.DigitalRead(cfg_.pinDipAddressTx2) << 0);
}

void AppPrototypeRadioBox1::
OnRadioRXAvailable(uint8_t  /*srcAddr */,
                   uint8_t  protocolId,
                   uint8_t *buf,
                   uint8_t  bufSize)
{
    if (protocolId == cfg_.valProtocolId)
    {
        if (bufSize == sizeof(Message))
        {
            Message *msg = (Message *)buf;
            
            OnMessageReceived(*msg);
        }
    }
}

void AppPrototypeRadioBox1::
RadioTX(uint8_t *buf, uint8_t bufSize)
{
    rfLink_->SendTo(radioAddressTx_,
                    cfg_.valProtocolId,
                    buf,
                    bufSize);
}

void AppPrototypeRadioBox1::
OnRadioTXComplete()
{

}


//////////////////////////////////////////////////////////////////////
//
// External Event Monitoring
//
//////////////////////////////////////////////////////////////////////

void AppPrototypeRadioBox1::
StartButtonMonitoring()
{
    using BtnToFn = InterruptEventHandlerDelegate<AppPrototypeRadioBox1>;
    
    static BtnToFn btfAttention (this, &AppPrototypeRadioBox1::OnAttentionButton);
    static BtnToFn btfFreeToTalk(this, &AppPrototypeRadioBox1::OnFreeToTalkButton);
    static BtnToFn btfYes       (this, &AppPrototypeRadioBox1::OnYesButton);
    static BtnToFn btfNo        (this, &AppPrototypeRadioBox1::OnNoButton);
    static BtnToFn btfClear     (this, &AppPrototypeRadioBox1::OnClearButton);

    btfAttention.RegisterForInterruptEvent(cfg_.pinAttentionButton);
    btfFreeToTalk.RegisterForInterruptEvent(cfg_.pinFreeToTalkButton);
    btfYes.RegisterForInterruptEvent(cfg_.pinYesButton);
    btfNo.RegisterForInterruptEvent(cfg_.pinNoButton);
    btfClear.RegisterForInterruptEvent(cfg_.pinClearButton);
}

void AppPrototypeRadioBox1::
StartRxTxAddressMonitoring()
{
    #if 0
    MapAndStartInterrupt(cfg_.pinDipAddressRx1,
                         this,
                         &AppPrototypeRadioBox1::OnRxTxAddressChange,
                         LEVEL_RISING_AND_FALLING);
    MapAndStartInterrupt(cfg_.pinDipAddressRx2,
                         this,
                         &AppPrototypeRadioBox1::OnRxTxAddressChange,
                         LEVEL_RISING_AND_FALLING);
    MapAndStartInterrupt(cfg_.pinDipAddressTx1,
                         this,
                         &AppPrototypeRadioBox1::OnRxTxAddressChange,
                         LEVEL_RISING_AND_FALLING);
    MapAndStartInterrupt(cfg_.pinDipAddressTx2,
                         this,
                         &AppPrototypeRadioBox1::OnRxTxAddressChange,
                         LEVEL_RISING_AND_FALLING);
                         
    #endif
}


















