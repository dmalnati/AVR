#include <Evm.h>
#include <LedFader.h>


static const uint8_t C_IDLE  = 1;
static const uint8_t C_TIMED = 1;
static const uint8_t C_INTER = 1;

Evm::Instance<C_IDLE,C_TIMED,C_INTER> evm;

RGBLEDFader<2> rgbLedFader;

void loop()
{
    uint8_t pinRed   = 15;
    uint8_t pinGreen = 16;
    uint8_t pinBlue  = 17;

    uint8_t pinFtt = 12;
    uint8_t pinYes = 11;
    uint8_t pinNo  =  5;

    rgbLedFader.AddLED(pinRed, pinGreen, pinBlue);
    rgbLedFader.AddLED(pinFtt, pinYes, pinNo);
    rgbLedFader.FadeForever();


    Evm::GetInstance().MainLoop();  // this works
    //evm.MainLoop();   // this also works
}



void setup()
{
}
