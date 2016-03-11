#include <Evm.h>
#include <AppPrototypeRadioBox1.h>


static AppPrototypeRadioBox1Config cfg = {
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
    .valProtocolId        = 240,

    // Radio Link Layer
    .valRealm             = 24,
    .pinDipAddressRx1     = 23,
    .pinDipAddressRx2     = 24,
    .pinDipAddressTx1     = 25,
    .pinDipAddressTx2     = 26,

    // Radio Physical Layer
    .pinRadioRX           =  2,
    .pinRadioTX           =  3,
    .valBaud              = 2000
};

static AppPrototypeRadioBox1 app(cfg);


void setup()
{
    app.Run();
}




void loop()
{
    
}





