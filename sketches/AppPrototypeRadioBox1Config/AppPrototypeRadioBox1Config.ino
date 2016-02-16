#include <AppPrototypeRadioBox1.h>


void loop()
{
    // Physical pins
    AppPrototypeRadioBox1Config cfg = {
        .pinDipAddressRx1     = 23,
        .pinDipAddressRx2     = 24,
        .pinDipAddressTx1     = 25,
        .pinDipAddressTx2     = 26,

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

        .pinRadioRX           =  3,
        .pinRadioTX           =  2
    };
    
    AppPrototypeRadioBox1 app(cfg);

    app.Run();
}



void setup()
{
    // Nothing to do
}







