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
, rfLink_()
, radioAddressRx_(0)
, radioAddressTx_(0)
, ptfAttention_    (this, &AppPrototypeRadioBox1::OnAttentionButton)
, ptfFreeToTalk_   (this, &AppPrototypeRadioBox1::OnFreeToTalkButton)
, ptfYes_          (this, &AppPrototypeRadioBox1::OnYesButton)
, ptfNo_           (this, &AppPrototypeRadioBox1::OnNoButton)
, ptfClear_        (this, &AppPrototypeRadioBox1::OnClearButton)
, ptfDipAddressRx1_(this, &AppPrototypeRadioBox1::OnRxTxAddressChange)
, ptfDipAddressRx2_(this, &AppPrototypeRadioBox1::OnRxTxAddressChange)
, ptfDipAddressTx1_(this, &AppPrototypeRadioBox1::OnRxTxAddressChange)
, ptfDipAddressTx2_(this, &AppPrototypeRadioBox1::OnRxTxAddressChange)
{
    // Nothing to do
}

AppPrototypeRadioBox1::
~AppPrototypeRadioBox1()
{
    // Nothing to do
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
    
    // Set up before running
    ApplicationSetup();
    
    // Start watching buttons and address change events
    StartAllMonitoring();
    
    // Enable Radio
    StartRadioSystem();
    
    // Begin application
    evm_.MainLoop();
}




//////////////////////////////////////////////////////////////////////
//
// Application Messaging System
//
//////////////////////////////////////////////////////////////////////


void AppPrototypeRadioBox1::
ApplicationSetup()
{
    // Associate the LED pins to the Faders
    ledFaderAttentionRedLED_.  AddLED(cfg_.pinAttentionRedLED,     0);
    ledFaderAttentionGreenLED_.AddLED(cfg_.pinAttentionGreenLED, 120);
    ledFaderAttentionBlueLED_. AddLED(cfg_.pinAttentionBlueLED,  240);
    ledFaderFreeToTalkLED_.    AddLED(cfg_.pinFreeToTalkLED);
    ledFaderYesLED_.           AddLED(cfg_.pinYesLED);
    ledFaderNoLED_.            AddLED(cfg_.pinNoLED);
}

void AppPrototypeRadioBox1::
CreateAndSendMessageByType(MessageType msgType)
{
    Message msg = { .msgType = msgType };
    
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
            ledFaderAttentionRedLED_.  FadeOnce(DEFAULT_FADER_DURATION_MS);
            ledFaderAttentionGreenLED_.FadeOnce(DEFAULT_FADER_DURATION_MS);
            ledFaderAttentionBlueLED_. FadeOnce(DEFAULT_FADER_DURATION_MS);
            
            break;
        }
        case MessageType::MSG_FREE_TO_TALK:
        {
            ledFaderFreeToTalkLED_.FadeOnce(DEFAULT_FADER_DURATION_MS);
            
            break;
        }
        case MessageType::MSG_YES:
        {
            ledFaderYesLED_.FadeOnce(DEFAULT_FADER_DURATION_MS);
            
            break;
        }
        case MessageType::MSG_NO:
        {
            ledFaderNoLED_.FadeOnce(DEFAULT_FADER_DURATION_MS);
            
            break;
        }
        
        default: break;
    }
}



//////////////////////////////////////////////////////////////////////
//
// Radio Link Layer Control System
//
//////////////////////////////////////////////////////////////////////

void AppPrototypeRadioBox1::
StartRadioSystem()
{
    // Start Radio Handler
    rfLink_.Init(
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

void AppPrototypeRadioBox1::
OnRxTxAddressChange(uint8_t /* logicLevel */)
{
    PAL.SoftReset();
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
    rfLink_.SendTo(radioAddressTx_, cfg_.valProtocolId, buf, bufSize);
}

void AppPrototypeRadioBox1::
OnRadioTXComplete()
{
    // Nothing to do
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
    
    PAL.Delay(DEFAULT_DELAY_BEFORE_ADDR_DISPLAY_MS);
    
    ShowConfiguredRadioAddressRxTx();
}

void AppPrototypeRadioBox1::
ShowLedFadeStartupSequence()
{
    for (uint8_t i = 0; i < 2; ++i)
    {
        AnimateFadeLeftToRight(DEFAULT_FADER_DURATION_MS);
        AnimateFadeRightToLeft(DEFAULT_FADER_DURATION_MS);
    }
}

void AppPrototypeRadioBox1::
AnimateFadeLeftToRight(uint32_t durationMs)
{
    AnimateFadeSweep(durationMs, StartupFadeDirection::LeftToRight);
}

void AppPrototypeRadioBox1::
AnimateFadeRightToLeft(uint32_t durationMs)
{
    AnimateFadeSweep(durationMs, StartupFadeDirection::RightToLeft);
}

void AppPrototypeRadioBox1::
AnimateFadeSweep(uint32_t             durationMs,
                 StartupFadeDirection fadeDirection)
{
    uint8_t phaseGroup1;
    uint8_t phaseGroup2;
    uint8_t phaseGroup3;
    
    if (fadeDirection == StartupFadeDirection::LeftToRight)
    {
        phaseGroup1 =   0;
        phaseGroup2 = 120;
        phaseGroup3 = 240;
    }
    else // (fadeDirection == RightToLeft)
    {
        phaseGroup1 = 240;
        phaseGroup2 = 120;
        phaseGroup3 =   0;
    }
    
    // Empty faders
    ledFaderStartup1_.ResetAndEmpty();
    ledFaderStartup2_.ResetAndEmpty();
    ledFaderStartup3_.ResetAndEmpty();
    
    // Rebuild faders
    ledFaderStartup1_.AddLED(cfg_.pinAttentionRedLED,   phaseGroup1);
    ledFaderStartup1_.AddLED(cfg_.pinFreeToTalkLED,     phaseGroup1);
    ledFaderStartup2_.AddLED(cfg_.pinAttentionGreenLED, phaseGroup2);
    ledFaderStartup2_.AddLED(cfg_.pinYesLED,            phaseGroup2);
    ledFaderStartup3_.AddLED(cfg_.pinAttentionBlueLED,  phaseGroup3);
    ledFaderStartup3_.AddLED(cfg_.pinNoLED,             phaseGroup3);
    
    // Given we know the faders are out of phase, scale the group durations
    // so that the last one finishes at the requested total duration.
    uint16_t durationGroupMs = (uint16_t)((double)durationMs / 1.6);

    // Start the faders
    ledFaderStartup1_.FadeOnce(durationGroupMs);
    ledFaderStartup2_.FadeOnce(durationGroupMs);
    ledFaderStartup3_.FadeOnce(durationGroupMs);
 
    // Let animation run
    evm_.HoldStackDangerously(durationMs);

    // Empty faders
    ledFaderStartup1_.ResetAndEmpty();
    ledFaderStartup2_.ResetAndEmpty();
    ledFaderStartup3_.ResetAndEmpty();
}

void AppPrototypeRadioBox1::
ShowConfiguredRadioAddressRxTx()
{
    ShowRadioAddressRxTx(radioAddressRx_, radioAddressTx_);
}

void AppPrototypeRadioBox1::
ShowRadioAddressRxTx(uint8_t addressRx, uint8_t addressTx)
{
    // Empty fader
    ledFaderRadioAddress_.ResetAndEmpty();
    
    // Rebuild the fader only with pins which are active
    if (addressRx & 0x02) { ledFaderRadioAddress_.AddLED(cfg_.pinAttentionBlueLED); }
    if (addressRx & 0x01) { ledFaderRadioAddress_.AddLED(cfg_.pinFreeToTalkLED);    }
    if (addressTx & 0x02) { ledFaderRadioAddress_.AddLED(cfg_.pinYesLED);           }
    if (addressTx & 0x01) { ledFaderRadioAddress_.AddLED(cfg_.pinNoLED);            }
    
    // Start the fader
    ledFaderRadioAddress_.FadeOnce(DEFAULT_FADER_DURATION_MS);
    
    // Let the animation run
    evm_.HoldStackDangerously(DEFAULT_FADER_DURATION_MS);

    // Empty the fader
    ledFaderRadioAddress_.ResetAndEmpty();
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
    CreateAndSendMessageByType(MessageType::MSG_NO);
}

void AppPrototypeRadioBox1::
OnClearButton(uint8_t)
{
    ShowConfiguredRadioAddressRxTx();
}



//////////////////////////////////////////////////////////////////////
//
// External Event Monitoring
//
//////////////////////////////////////////////////////////////////////

void AppPrototypeRadioBox1::
StartAllMonitoring()
{
    StartButtonMonitoring();
    StartRxTxAddressMonitoring();
}

void AppPrototypeRadioBox1::
StopAllMonitoring()
{
    StopButtonMonitoring();
    StopRxTxAddressMonitoring();
}

void AppPrototypeRadioBox1::
StartButtonMonitoring()
{
    ptfAttention_. RegisterForInterruptEvent(cfg_.pinAttentionButton);
    ptfFreeToTalk_.RegisterForInterruptEvent(cfg_.pinFreeToTalkButton);
    ptfYes_.       RegisterForInterruptEvent(cfg_.pinYesButton);
    ptfNo_.        RegisterForInterruptEvent(cfg_.pinNoButton);
    ptfClear_.     RegisterForInterruptEvent(cfg_.pinClearButton);
}

void AppPrototypeRadioBox1::
StopButtonMonitoring()
{
    ptfAttention_. DeRegisterForInterruptEvent();
    ptfFreeToTalk_.DeRegisterForInterruptEvent();
    ptfYes_.       DeRegisterForInterruptEvent();
    ptfNo_.        DeRegisterForInterruptEvent();
    ptfClear_.     DeRegisterForInterruptEvent();
}

void AppPrototypeRadioBox1::
StartRxTxAddressMonitoring()
{
    ptfDipAddressRx1_.RegisterForInterruptEvent(cfg_.pinDipAddressRx1,
                                                LEVEL_RISING_AND_FALLING);
    ptfDipAddressRx2_.RegisterForInterruptEvent(cfg_.pinDipAddressRx2,
                                                LEVEL_RISING_AND_FALLING);
    ptfDipAddressTx1_.RegisterForInterruptEvent(cfg_.pinDipAddressTx1,
                                                LEVEL_RISING_AND_FALLING);
    ptfDipAddressTx2_.RegisterForInterruptEvent(cfg_.pinDipAddressTx2,
                                                LEVEL_RISING_AND_FALLING);
}

void AppPrototypeRadioBox1::
StopRxTxAddressMonitoring()
{
    ptfDipAddressRx1_.DeRegisterForInterruptEvent();
    ptfDipAddressRx2_.DeRegisterForInterruptEvent();
    ptfDipAddressTx1_.DeRegisterForInterruptEvent();
    ptfDipAddressTx2_.DeRegisterForInterruptEvent();
}


















