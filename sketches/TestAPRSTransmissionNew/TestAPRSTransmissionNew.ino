#include "PAL.h"
#include "AX25UIMessage.h"
#include "ModemBell202.h"


static const uint8_t BUF_SIZE = 60;
static uint8_t bufShared[BUF_SIZE] = { 0 };

static AX25UIMessage msg;
static ModemBell202 *modem;


void Send(uint8_t *buf, uint8_t bufLen)
{
    // APRSDroid sends ~256 flags to start...
    uint8_t flagList[]  = { 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E };
    uint8_t flagListLen = sizeof(flagList);

    uint8_t bitStuff = 0;

    modem->Start();

    // Send preamble, which also will serve as the flag byte
    // Send lots, like APRSDroid
    for (uint8_t i = 0; i < 3; ++i)
    {
        bitStuff = 0;
        modem->Send(flagList, flagListLen, bitStuff);
    }

    // send content
    bitStuff = 1;
    modem->Send(buf, bufLen, bitStuff);

    // Send trailing flags
    bitStuff = 0;
    modem->Send(flagList, 3, bitStuff);

    modem->Stop();
}

void Test()
{
    uint8_t *buf = bufShared;

    // Create buffer the way as seen in the example checksum page
    // http://practicingelectronics.com/articles/article-100003/article.php
    msg.Init(buf);

    //msg.SetDstAddress("CQ",     0);
    msg.SetDstAddress("KD2KDD",     6);
    msg.SetSrcAddress("KB2BRD", 2);

    msg.AppendInfo((uint8_t *)"A\r", 2);

    // hack in the has-been-repeated bit on the final address,
    // as was seen in the example on the website
    //buf[13] |= 0b10000000;
    
    uint8_t bytesUsed = msg.Finalize();


    Send(buf, bytesUsed);
}



void Test2()
{
    uint8_t *buf = bufShared;
    
    msg.Init(buf);

    msg.SetDstAddress("APDR14", 0);
    msg.SetSrcAddress("KD2KDD", 9);
    msg.AddRepeaterAddress("WIDE1", 1);

    const char *info = ":KD2KDD   :hi2{2";
    uint8_t infoLen = strlen(info);   // 16
    msg.AppendInfo((uint8_t *)info, infoLen);

    // hack in the has-been-repeated bit on the dst address,
    buf[6] |= 0b10000000;
    
    uint8_t bytesUsed = msg.Finalize();


    Send(buf, bytesUsed);
}


void setup()
{
    Serial.begin(9600);

    // So I can use Serial in the class for debugging...
    ModemBell202 modemReal;
    modem = &modemReal;

    modem->Init();

    while (1)
    {
        Test2();
        PAL.Delay(1000);
    }
}



void loop() { }












