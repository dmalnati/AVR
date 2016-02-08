#include <SoftwareSerial.h>
#include <Evm.h>
#include <MyLedFader.h>

const uint8_t PIN_SERIAL_TX = 1;
const uint8_t PIN_SERIAL_RX = -1;

SoftwareSerial sS(PIN_SERIAL_RX, PIN_SERIAL_TX);

void setup()
{
    pinMode(PIN_SERIAL_TX, OUTPUT);
    sS.begin(9600);
}

void loop()
{
    Evm &evm = Evm::GetInstance();

    MyLedFader f0(0);
    MyLedFader f2(2);
    MyLedFader f3(3);
    MyLedFader f4(4);

    f0.Fade();
    f2.Fade();
    f3.Fade();
    f4.Fade();
    

    evm.MainLoop();
}
