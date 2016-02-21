#include <util/atomic.h>

#include "AppPrototypeRadioBox1.h"


//////////////////////////////////////////////////////////////////////
//
// Constructor and Destructor
//
//////////////////////////////////////////////////////////////////////

AppPrototypeRadioBox1::
AppPrototypeRadioBox1(AppPrototypeRadioBox1Config &cfg)
: cfg_(cfg)
, rfLink_(NULL)
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
// Visual Interface System
//
//////////////////////////////////////////////////////////////////////

void PinToggle(uint8_t pin)
{
    PAL.PinMode(pin, OUTPUT);
    
    PAL.DigitalWrite(pin, HIGH);
    PAL.Delay(500);
    PAL.DigitalWrite(pin, LOW);
}


/*
void AppPrototypeRadioBox1::
StartupLightShow()
{
    // Set up LEDs as output
    PAL.PinMode(cfg_.pinAttentionRedLED,   OUTPUT);
    PAL.PinMode(cfg_.pinAttentionGreenLED, OUTPUT);
    PAL.PinMode(cfg_.pinAttentionBlueLED,  OUTPUT);
    PAL.PinMode(cfg_.pinFreeToTalkLED,     OUTPUT);
    PAL.PinMode(cfg_.pinYesLED,            OUTPUT);
    PAL.PinMode(cfg_.pinNoLED,             OUTPUT);
    
    // Showtime
    PinToggle(cfg_.pinAttentionRedLED);
    PinToggle(cfg_.pinAttentionGreenLED);
    PinToggle(cfg_.pinAttentionBlueLED);
    PinToggle(cfg_.pinFreeToTalkLED);
    PinToggle(cfg_.pinYesLED);
    PinToggle(cfg_.pinNoLED);
    
    PAL.Delay(500);
    
    ShowRadioAddressRxTx();
    
    PAL.Delay(500);
}
*/

void AppPrototypeRadioBox1::
StartupLightShow()
{
    ledFader_.AddLED(cfg_.pinAttentionRedLED,    90);
    ledFader_.AddLED(cfg_.pinAttentionGreenLED, 210);
    ledFader_.AddLED(cfg_.pinAttentionBlueLED,  330);
    
    ledFader_.AddLED(cfg_.pinFreeToTalkLED,  120);
    ledFader_.AddLED(cfg_.pinYesLED,        180);
    ledFader_.AddLED(cfg_.pinNoLED,         270);
    
    ledFader_.FadeForever(1000);
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
    
    PAL.Delay(500);
    
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
}

void AppPrototypeRadioBox1::
StartRxTxAddressMonitoring()
{   
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
}


















