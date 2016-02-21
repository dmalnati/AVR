#include <AppPrototypeRadioBox1.h>




// Debug
#include <EvmEventHandlerUtils.h>
#include <LEDFader.h>


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
        .pinRadioRX           =  3,
        .pinRadioTX           =  2,
        .valBaud              = 2000
    };
    
    AppPrototypeRadioBox1 app(cfg);


    // Debug
    //(new IdlePinToggler(cfg.pinNoLED))->RegisterForIdleTimeEvent();

    LEDFader fader;

    // Set up Attention LEDs
    fader.AddLED(cfg.pinAttentionRedLED);
    fader.AddLED(cfg.pinAttentionGreenLED, 90);
    fader.AddLED(cfg.pinAttentionBlueLED, 180);

    // Set up Free To Talk LEDs
    //fader.AddLED(cfg.pinFreeToTalkLED, 270);

    // Engage
    //fader.FadeOnce(3000);


    app.Run();
}



void setup()
{
    // Nothing to do
}







