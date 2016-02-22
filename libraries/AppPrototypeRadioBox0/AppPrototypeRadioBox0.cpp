#include <util/atomic.h>

#include "Utl.h"
#include "Evm.h"
#include "AppPrototypeRadioBox0.h"








//////////////////////////////////////////////////////////////////////
//
// Constructor and Destructor
//
//////////////////////////////////////////////////////////////////////

AppPrototypeRadioBox0::
AppPrototypeRadioBox0(AppPrototypeRadioBox0Config &cfg)
: cfg_(cfg)
, radioAddressRx_(0)
, radioAddressTx_(0)
{
    PinToggle(cfg_.pinFreeToTalkLED);
    
    // Read in dynamic configuration
    ReadRadioAddressRxTx();
    
    PinToggle(cfg_.pinFreeToTalkLED);
    
    // Dazzle
    StartupLightShow();
}

AppPrototypeRadioBox0::
~AppPrototypeRadioBox0()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
    }
}



//////////////////////////////////////////////////////////////////////
//
// Run
//
//////////////////////////////////////////////////////////////////////

void AppPrototypeRadioBox0::
Run()
{
    PinToggle(cfg_.pinFreeToTalkButton);
    Evm::GetInstance().MainLoop();
    PinToggle(cfg_.pinFreeToTalkButton);
}


//////////////////////////////////////////////////////////////////////
//
// Visual Interface System
//
//////////////////////////////////////////////////////////////////////


void AppPrototypeRadioBox0::
StartupLightShow()
{
    ShowRadioAddressRxTx();
}



void AppPrototypeRadioBox0::
ShowRadioAddressRxTx()
{
    PinToggle(cfg_.pinYesButton);
    
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
    
    PinToggle(cfg_.pinYesButton);
}


//////////////////////////////////////////////////////////////////////
//
// Radio Control System
//
//////////////////////////////////////////////////////////////////////


void AppPrototypeRadioBox0::
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

















