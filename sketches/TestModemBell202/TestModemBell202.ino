#include "PAL.h"
#include "ModemBell202.h"


static ModemBell202 m;



void TestSendJustZeros()
{
    uint8_t buf[] = { 0b00000000 };

    m.Start();

    m.Send(buf, 1);
    
    m.Stop();
}



void TestSendJustOnes()
{
    uint8_t buf[] = { 0b11111111 };

    m.Start();

    m.Send(buf, 1);
    
    m.Stop();
}

void TestSendJustOnesNoStuff()
{
    uint8_t buf[] = { 0b11111111 };

    m.Start();

    m.Send(buf, 1, 0);
    
    m.Stop();
}


void setup()
{
    Serial.begin(9600);
    
    m.Init();

    while (1)
    {
        TestSendJustZeros();
        PAL.Delay(500);
        
        TestSendJustOnes();
        PAL.Delay(500);

        TestSendJustOnesNoStuff();
        PAL.Delay(500);
    }
}






void loop() {}



