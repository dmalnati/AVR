
#include <Utl.h>
#include <Evm.h>
#include <AppPrototypeRadioBox0.h>


const uint8_t PIN_NO = 5;
const uint8_t PIN_YES = 11;
const uint8_t PIN_FTT = 12;


class Test
{
public:
    Test(AppPrototypeRadioBox0Config &cfg)
    {
        PinToggle(cfg.pinFreeToTalkLED);
    }

    void Run()
    {
        PinToggle(PIN_FTT);
        Evm::GetInstance().MainLoop();
    }

private:

};



void setup()
{

}

void loop()
{
    PinToggle(PIN_NO);






    // Physical pins
    AppPrototypeRadioBox0Config cfg = {

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




    
    
    Test t(cfg);

    PinToggle(PIN_NO);

    AppPrototypeRadioBox0 app(cfg);

    PinToggle(PIN_NO);

    //t.Run();
    app.Run();
}












