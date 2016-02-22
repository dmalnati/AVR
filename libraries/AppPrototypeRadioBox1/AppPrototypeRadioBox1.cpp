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
: cfg_(cfg)
//, rfLink_(NULL)
, radioAddressRx_(0)
, radioAddressTx_(0)
{
    // Read in dynamic configuration
    ReadRadioAddressRxTx();
    
    // Dazzle
    StartupLightShow();
    
    // Enable Radio
    StartRadioSystem();
    
    // Start watching buttons
    StartButtonMonitoring();
    
    
    
        PinToggle(cfg_.pinFreeToTalkLED);
    

}

AppPrototypeRadioBox1::
~AppPrototypeRadioBox1()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        //delete rfLink_;
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
    PinToggle(cfg_.pinFreeToTalkLED);
    
    Evm::GetInstance().MainLoop();
}


//////////////////////////////////////////////////////////////////////
//
// Visual Interface System
//
//////////////////////////////////////////////////////////////////////


void AppPrototypeRadioBox1::
StartupLightShow()
{
    PinToggle(cfg_.pinFreeToTalkLED);
    ShowRadioAddressRxTx();
}


void AppPrototypeRadioBox1::
ShowRadioAddressRxTx()
{
    PinToggle(cfg_.pinFreeToTalkLED);
    
    PAL.DigitalWrite(cfg_.pinAttentionBlueLED,
                     radioAddressRx_ & 0x02 ? HIGH : LOW);
    PAL.DigitalWrite(cfg_.pinFreeToTalkLED,
                     radioAddressRx_ & 0x01 ? HIGH : LOW);
    PAL.DigitalWrite(cfg_.pinYesLED, 
                     radioAddressTx_ & 0x02 ? HIGH : LOW);
    PAL.DigitalWrite(cfg_.pinNoLED,
                     radioAddressTx_ & 0x01 ? HIGH : LOW);
    
    PAL.Delay(500);
    
    PAL.DigitalWrite(cfg_.pinAttentionBlueLED, LOW);
    PAL.DigitalWrite(cfg_.pinFreeToTalkLED,    LOW);
    PAL.DigitalWrite(cfg_.pinYesLED,           LOW);
    PAL.DigitalWrite(cfg_.pinNoLED,            LOW);
    
    PinToggle(cfg_.pinFreeToTalkLED);
}


//////////////////////////////////////////////////////////////////////
//
// Button Interface System
//
//////////////////////////////////////////////////////////////////////

void AppPrototypeRadioBox1::
OnAttentionButton(uint8_t logicLevel)
{
    CreateAndSendMessageByType(MessageType::MSG_ATTENTION);
}


void AppPrototypeRadioBox1::
OnFreeToTalkButton(uint8_t logicLevel)
{
    CreateAndSendMessageByType(MessageType::MSG_FREE_TO_TALK);
}

void AppPrototypeRadioBox1::
OnYesButton(uint8_t logicLevel)
{
    CreateAndSendMessageByType(MessageType::MSG_YES);
}

void AppPrototypeRadioBox1::
OnNoButton(uint8_t logicLevel)
{
    CreateAndSendMessageByType(MessageType::MSG_NO);
}

void AppPrototypeRadioBox1::
OnClearButton(uint8_t logicLevel)
{
    PinToggle(cfg_.pinNoButton);
}


//////////////////////////////////////////////////////////////////////
//
// Application Messaging System
//
//////////////////////////////////////////////////////////////////////

void AppPrototypeRadioBox1::
CreateAndSendMessageByType(MessageType msgType)
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
    
    return;
    
    
    
    // Monitor for changes to the RX or TX Address
    StartRxTxAddressMonitoring();
    
    #if 0
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
    #endif
}

void AppPrototypeRadioBox1::
OnRxTxAddressChange(uint8_t /* logicLevel */)
{
    ReadRadioAddressRxTx();
    ShowRadioAddressRxTx();
    
    #if 0
    // Keep the radio system aware of the local address
    if (rfLink_)
    {
        rfLink_->SetSrcAddr(radioAddressRx_);
    }
    #endif
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
OnRadioRXAvailable(uint8_t  srcAddr,
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
    #if 0
    rfLink_->SendTo(radioAddressTx_,
                    cfg_.valProtocolId,
                    buf,
                    bufSize);
    #endif
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
    
    
    
        MapAndStartInterrupt(cfg_.pinClearButton,
                         this,
                         &AppPrototypeRadioBox1::OnClearButton);
    
    
    
    #if 0
    MapAndStartInterrupt(cfg_.pinAttentionButton,
                         this,
                         &AppPrototypeRadioBox1::OnAttentionButton);
    MapAndStartInterrupt(cfg_.pinFreeToTalkButton,
                         this,
                         &AppPrototypeRadioBox1::OnFreeToTalkButton);
    MapAndStartInterrupt(cfg_.pinYesButton,
                         this,
                         &AppPrototypeRadioBox1::OnYesButton);
    MapAndStartInterrupt(cfg_.pinNoButton,
                         this,
                         &AppPrototypeRadioBox1::OnNoButton);
    MapAndStartInterrupt(cfg_.pinClearButton,
                         this,
                         &AppPrototypeRadioBox1::OnClearButton);
    #endif
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


















