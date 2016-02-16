#include "AppPrototypeRadioBox1.h"






void PinToggle(uint8_t pin)
{
    PAL.PinMode(pin, OUTPUT);
    
    PAL.DigitalWrite(pin, HIGH);
    PAL.Delay(500);
    PAL.DigitalWrite(pin, LOW);
}










// Debug
#include <EvmEventHandlerUtils.h>


AppPrototypeRadioBox1::
AppPrototypeRadioBox1(AppPrototypeRadioBox1Config &cfg)
: cfg_(cfg)
, rfLink_(NULL)
, radioAddressRx_(0)
, radioAddressTx_(0)
{
    // Debug
    (new IdlePinToggler(cfg_.pinNoLED))->RegisterForIdleTimeEvent();;
    
    
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
    delete rfLink_;
}



void AppPrototypeRadioBox1::
StartRadioSystem()
{
    // Monitor for changes to the RX or TX Address
    StartRxTxAddressMonitoring();
    
    // Start Radio Handler
    rfLink_ =
        new RFLink<AppPrototypeRadioBox1>(
            this,
            cfg_.pinRadioRX,
            &AppPrototypeRadioBox1::OnRadioRXAvailable,
            cfg_.pinRadioTX,
            &AppPrototypeRadioBox1::OnRadioTXComplete
        );
}

void AppPrototypeRadioBox1::
OnRadioRXAvailable(uint8_t *buf, uint8_t bufSize)
{
    
}

void AppPrototypeRadioBox1::
OnRadioTXComplete()
{
    // TX complete from Clear button
    PAL.DigitalWrite(cfg_.pinYesLED, LOW);
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

void AppPrototypeRadioBox1::
StartRxTxAddressMonitoring()
{
    MapAndStartInterrupt(cfg_.pinDipAddressRx1,
                         this,
                         &AppPrototypeRadioBox1::OnDipAddressRx1,
                         LEVEL_RISING_AND_FALLING);
    MapAndStartInterrupt(cfg_.pinDipAddressRx2,
                         this,
                         &AppPrototypeRadioBox1::OnDipAddressRx2,
                         LEVEL_RISING_AND_FALLING);
    MapAndStartInterrupt(cfg_.pinDipAddressTx1,
                         this,
                         &AppPrototypeRadioBox1::OnDipAddressTx1,
                         LEVEL_RISING_AND_FALLING);
    MapAndStartInterrupt(cfg_.pinDipAddressTx2,
                         this,
                         &AppPrototypeRadioBox1::OnDipAddressTx2,
                         LEVEL_RISING_AND_FALLING);
}

void AppPrototypeRadioBox1::
OnDipAddressRx1(uint8_t logicLevel)
{
    ReadRadioAddressRxTx();
    ShowRadioAddressRxTx();
}

void AppPrototypeRadioBox1::
OnDipAddressRx2(uint8_t logicLevel)
{
    ReadRadioAddressRxTx();
    ShowRadioAddressRxTx();
}

void AppPrototypeRadioBox1::
OnDipAddressTx1(uint8_t logicLevel)
{
    ReadRadioAddressRxTx();
    ShowRadioAddressRxTx();
}

void AppPrototypeRadioBox1::
OnDipAddressTx2(uint8_t logicLevel)
{
    ReadRadioAddressRxTx();
    ShowRadioAddressRxTx();
}



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
OnAttentionButton(uint8_t logicLevel)
{
    PinToggle(cfg_.pinAttentionRedLED);
}


void AppPrototypeRadioBox1::
OnFreeToTalkButton(uint8_t logicLevel)
{
    PinToggle(cfg_.pinAttentionGreenLED);
}

void AppPrototypeRadioBox1::
OnYesButton(uint8_t logicLevel)
{
    PinToggle(cfg_.pinAttentionBlueLED);
}

void AppPrototypeRadioBox1::
OnNoButton(uint8_t logicLevel)
{
    
}

void AppPrototypeRadioBox1::
OnClearButton(uint8_t logicLevel)
{
    PAL.DigitalWrite(cfg_.pinYesLED, HIGH);
    
    uint8_t buf[5] = { 1, 2, 3, 4, 5 };
    rfLink_->Send(buf, 5);
}























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







