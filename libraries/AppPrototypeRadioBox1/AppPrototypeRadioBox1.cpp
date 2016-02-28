#include "AppPrototypeRadioBox1.h"


//////////////////////////////////////////////////////////////////////
//
// Constructor and Destructor
//
//////////////////////////////////////////////////////////////////////

AppPrototypeRadioBox1::
AppPrototypeRadioBox1(AppPrototypeRadioBox1Config &cfg)
: cfg_(cfg)
, radioAddressRx_(0)
, radioAddressTx_(0)
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
// Application Logic
//
//////////////////////////////////////////////////////////////////////

void AppPrototypeRadioBox1::
ApplicationSetup()
{
    // Set up timeout object for long-press on Clear button to Reset
    rot_.SetCallback(&PAL, &PlatformAbstractionLayer::SoftReset);
    
    // Associate the LED pins to the Faders
    ledFaderAttentionRedLED_.  AddLED(cfg_.pinAttentionRedLED,     0);
    ledFaderAttentionGreenLED_.AddLED(cfg_.pinAttentionGreenLED, 120);
    ledFaderAttentionBlueLED_. AddLED(cfg_.pinAttentionBlueLED,  240);
    
    ledFaderFreeToTalkLED_.    AddLED(cfg_.pinFreeToTalkLED);
    ledFaderYesLED_.           AddLED(cfg_.pinYesLED);
    ledFaderNoLED_.            AddLED(cfg_.pinNoLED);
}




void AppPrototypeRadioBox1::
OnRxTxAddressChange(uint8_t /* logicLevel */)
{
    PAL.SoftReset();
}




//
// Attention
//
void AppPrototypeRadioBox1::
OnAttentionButton(uint8_t)
{
    StopAttentionFaders();
    StartAttentionFadersOnce();
    
    CreateAndSendMessageByType(MessageType::MSG_ATTENTION);
}

void AppPrototypeRadioBox1::
OnMsgAttention()
{
    StartAttentionFadersForever();
}

//
// Free to talk
//
void AppPrototypeRadioBox1::
OnFreeToTalkButton(uint8_t)
{
    // If already blinking, then pressing does nothing.
    
    // If a yes/no answer is still displayed, don't send, must clear first.
    
    if (!IsActiveYes() && !IsActiveNo())
    {
        StopYesFaders();
        StopNoFaders();
        
        if (!IsActiveFreeToTalk())
        {
            StartFreeToTalkFadersOnce();
            
            CreateAndSendMessageByType(MessageType::MSG_FREE_TO_TALK);
        }
    }
}

void AppPrototypeRadioBox1::
OnMsgFreeToTalk()
{
    StopYesFaders();
    StopNoFaders();
    
    StartFreeToTalkFadersForever();
}

//
// Yes
//
void AppPrototypeRadioBox1::
OnYesButton(uint8_t)
{
    if (IsActiveYes() || IsActiveNo())
    {
        // Do nothing
    }
    else
    {
        StopFreeToTalkFaders();
        StopNoFaders();
        StartYesFadersOnce();
        
        CreateAndSendMessageByType(MessageType::MSG_YES);
    }
}

void AppPrototypeRadioBox1::
OnMsgYes()
{
    // If FreeToTalk is active, then this message inbound doesn'table
    // make sense, ignore it.
    if (!IsActiveFreeToTalk())
    {
        // The other side is allowed to change their answer.
        if (IsActiveNo())
        {
            StopNoFaders();
        }
        
        StartYesFadersForever();
    }
}

//
// No
//
void AppPrototypeRadioBox1::
OnNoButton(uint8_t)
{
    if (IsActiveNo() || IsActiveYes())
    {
        // Do nothing
    }
    else
    {
        StopFreeToTalkFaders();
        StopYesFaders();
        StartNoFadersOnce();
        
        CreateAndSendMessageByType(MessageType::MSG_NO);
    }
}

void AppPrototypeRadioBox1::
OnMsgNo()
{
    // If FreeToTalk is active, then this message inbound doesn't
    // make sense, ignore it.
    if (!IsActiveFreeToTalk())
    {
        if (IsActiveYes())
        {
            StopYesFaders();
        }
        
        StartNoFadersForever();
    }
}
    

//
// Clear
//
void AppPrototypeRadioBox1::
OnClearButtonPressed()
{
    // Stop all faders
    StopAttentionFaders();
    StopFreeToTalkFaders();
    StopYesFaders();
    StopNoFaders();
}

void AppPrototypeRadioBox1::
OnClearButton(uint8_t logicLevel)
{
    if (logicLevel == 1)
    {
        // Queue timer to reset if this is in fact a reset
        rot_.RegisterForTimedEvent(DEFAULT_HOLD_TO_RESET_DURATION_MS);
        
        OnClearButtonPressed();
    }
    else
    {
        // I guess it wasn't a reset.  Cancel timeout.
        rot_.DeRegisterForTimedEvent();
    }
}







//
// Start / Stop of Faders
//
void AppPrototypeRadioBox1::
StartAttentionFadersForever()
{
    ledFaderAttentionRedLED_.  FadeForever(DEFAULT_FADER_DURATION_MS);
    ledFaderAttentionGreenLED_.FadeForever(DEFAULT_FADER_DURATION_MS);
    ledFaderAttentionBlueLED_. FadeForever(DEFAULT_FADER_DURATION_MS);
}

void AppPrototypeRadioBox1::
StartAttentionFadersOnce()
{
    ledFaderAttentionRedLED_.  FadeOnce(DEFAULT_FADER_DURATION_MS);
    ledFaderAttentionGreenLED_.FadeOnce(DEFAULT_FADER_DURATION_MS);
    ledFaderAttentionBlueLED_. FadeOnce(DEFAULT_FADER_DURATION_MS);
}

void AppPrototypeRadioBox1::
StopAttentionFaders()
{
    ledFaderAttentionRedLED_.  Stop();
    ledFaderAttentionGreenLED_.Stop();
    ledFaderAttentionBlueLED_. Stop();
}

void AppPrototypeRadioBox1::
StartFreeToTalkFadersForever()
{
    ledFaderFreeToTalkLED_.FadeForever(DEFAULT_FADER_DURATION_MS);
}

void AppPrototypeRadioBox1::
StartFreeToTalkFadersOnce()
{
    ledFaderFreeToTalkLED_.FadeOnce(DEFAULT_FADER_DURATION_MS);
}

void AppPrototypeRadioBox1::
StopFreeToTalkFaders()
{
    ledFaderFreeToTalkLED_.Stop();
}

void AppPrototypeRadioBox1::
StartYesFadersForever()
{
    ledFaderYesLED_.FadeForever(DEFAULT_FADER_DURATION_MS);
}

void AppPrototypeRadioBox1::
StartYesFadersOnce()
{
    ledFaderYesLED_.FadeOnce(DEFAULT_FADER_DURATION_MS);
}

void AppPrototypeRadioBox1::
StopYesFaders()
{
    ledFaderYesLED_.Stop();
}

void AppPrototypeRadioBox1::
StartNoFadersForever()
{
    ledFaderNoLED_.FadeForever(DEFAULT_FADER_DURATION_MS);
}

void AppPrototypeRadioBox1::
StartNoFadersOnce()
{
    ledFaderNoLED_.FadeOnce(DEFAULT_FADER_DURATION_MS);
}

void AppPrototypeRadioBox1::
StopNoFaders()
{
    ledFaderNoLED_.Stop();
}


//
// Blink state checking
//
uint8_t AppPrototypeRadioBox1::
IsActiveFreeToTalk()
{
    return ledFaderFreeToTalkLED_.IsActive();
}

uint8_t AppPrototypeRadioBox1::
IsActiveYes()
{
    return ledFaderYesLED_.IsActive();
}

uint8_t AppPrototypeRadioBox1::
IsActiveNo()
{
    return ledFaderNoLED_.IsActive();
}


















//////////////////////////////////////////////////////////////////////
//
// Application Messaging Layer
//
//////////////////////////////////////////////////////////////////////

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
        case MessageType::MSG_ATTENTION:    { OnMsgAttention();   break; }
        case MessageType::MSG_FREE_TO_TALK: { OnMsgFreeToTalk();  break; }
        case MessageType::MSG_YES:          { OnMsgYes();         break; }
        case MessageType::MSG_NO:           { OnMsgNo();          break; }
        
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
    ptfAttention_.    SetCallback(this, &AppPrototypeRadioBox1::OnAttentionButton);
    ptfFreeToTalk_.   SetCallback(this, &AppPrototypeRadioBox1::OnFreeToTalkButton);
    ptfYes_.          SetCallback(this, &AppPrototypeRadioBox1::OnYesButton);
    ptfNo_.           SetCallback(this, &AppPrototypeRadioBox1::OnNoButton);
    ptfClear_.        SetCallback(this, &AppPrototypeRadioBox1::OnClearButton);
    
    ptfAttention_. RegisterForInterruptEvent(cfg_.pinAttentionButton);
    ptfFreeToTalk_.RegisterForInterruptEvent(cfg_.pinFreeToTalkButton);
    ptfYes_.       RegisterForInterruptEvent(cfg_.pinYesButton);
    ptfNo_.        RegisterForInterruptEvent(cfg_.pinNoButton);
    ptfClear_.     RegisterForInterruptEvent(cfg_.pinClearButton,
                                             LEVEL_RISING_AND_FALLING);
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
    ptfDipAddressRx1_.SetCallback(this, &AppPrototypeRadioBox1::OnRxTxAddressChange);
    ptfDipAddressRx2_.SetCallback(this, &AppPrototypeRadioBox1::OnRxTxAddressChange);
    ptfDipAddressTx1_.SetCallback(this, &AppPrototypeRadioBox1::OnRxTxAddressChange);
    ptfDipAddressTx2_.SetCallback(this, &AppPrototypeRadioBox1::OnRxTxAddressChange);
    
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


















