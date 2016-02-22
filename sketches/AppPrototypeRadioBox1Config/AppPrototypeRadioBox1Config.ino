#include <Utl.h>
#include <AppPrototypeRadioBox1.h>


// Debug
#include <Evm.h>
#include <LedFader.h>

void setup()
{
}


void loop()
{
    // Physical pins
    AppPrototypeRadioBox1Config cfg = {

        // Visual and Button Interface System
        .pinAttentionButton   = 18,
        .pinAttentionRedLED   = 15,
        .pinAttentionGreenLED = 16,
        .pinAttentionBlueLED  = 17,

        .pinFreeToTalkButton  = 13,
        .pinFreeToTalkLED     = 12,

        .pinYesButton         =  6,
        .pinYesLED            = 11,

        .pinNoButton          =  4,
        .pinNoLED             =  5,

        .pinClearButton       = 27,

        // Application Messaging System
        .valProtocolId        =  1,

        // Radio Link Layer
        .valRealm             =  1,
        .pinDipAddressRx1     = 23,
        .pinDipAddressRx2     = 24,
        .pinDipAddressTx1     = 25,
        .pinDipAddressTx2     = 26,

        // Radio Physical Layer
        .pinRadioRX           =  2,
        .pinRadioTX           =  3,
        .valBaud              = 2000
    };

    LEDFader fader;

    fader.AddLED(cfg.pinNoLED);
    fader.FadeForever();
    Evm::GetInstance().MainLoop();

    PinToggle(cfg.pinNoLED);
    
    AppPrototypeRadioBox1 app(cfg);
    
    PinToggle(cfg.pinNoLED);
    
    app.Run();
}








