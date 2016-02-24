#include <Evm.h>
#include <LedFader.h>





const uint8_t C_IDLE  = 1;
const uint8_t C_TIMED = 1;
const uint8_t C_INTER = 1;

void loop()
{
    Evm::CreateInstance<C_IDLE,C_TIMED,C_INTER>();

    RGBLEDFader<1> rgbLedFader;
    
    uint8_t pinRed   = 15;
    uint8_t pinGreen = 16;
    uint8_t pinBlue  = 17;

    rgbLedFader.AddLED(pinRed, pinGreen, pinBlue);
    rgbLedFader.FadeForever();



    Evm::GetInstance().MainLoop();
}



void setup()
{
}
