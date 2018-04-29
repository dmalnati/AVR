#include "PAL.h"
#include "UtlStreamBlob.h"
#include "AX25UIMessage.h"


// reproducing test cases found here:
// http://practicingelectronics.com/articles/article-100003/article.php


static const uint8_t BUF_SIZE = 60;
static uint8_t bufShared[BUF_SIZE] = { 0 };

static AX25UIMessage msg;


void setup()
{
    @fix@Serial.begin(9600);

    while (1)
    {
        DoMessageTest();
        PAL.Delay(1000);

        DoSimpleTestValuesTest();
        PAL.Delay(1000);

        DoCommonTestValueTest();
        PAL.Delay(1000);
    }
}

void DoCommonTestValueTest()
{
    const char *buf = "123456789";
    uint8_t bufLen = strlen(buf);

    @fix@Serial.print("CRCing string: \"");
    @fix@Serial.print(buf);
    @fix@Serial.print("\"");
    @fix@Serial.println();
    StreamBlob(Serial, (uint8_t *)buf, bufLen, 1);

    uint16_t crc = AX25UIMessage::calc_crc((uint8_t *)buf, bufLen);
    StreamBlob(Serial, (uint8_t *)&crc, sizeof(crc), 1);

    @fix@Serial.println();
    @fix@Serial.println();
}

void DoSimpleTestValuesTest()
{
    const char *buf = "ABC";
    uint8_t bufLen = strlen(buf);

    @fix@Serial.print("CRCing string: \"");
    @fix@Serial.print(buf);
    @fix@Serial.print("\"");
    @fix@Serial.println();
    StreamBlob(Serial, (uint8_t *)buf, bufLen, 1);

    uint16_t crc = AX25UIMessage::calc_crc((uint8_t *)buf, bufLen);
    StreamBlob(Serial, (uint8_t *)&crc, sizeof(crc), 1);

    @fix@Serial.println();
    @fix@Serial.println();
}

void DoMessageTest()
{
    uint8_t *buf     = bufShared;
    uint8_t  bufSize = BUF_SIZE;
    
    msg.Init(buf);

    msg.SetDstAddress("CQ",     0);
    msg.SetSrcAddress("KB2BRD", 2);

    @fix@Serial.println("Post SetAddress");
    StreamBlob(Serial, buf, bufSize, 1);

    msg.AppendInfo((uint8_t *)"A\r", 2);

    @fix@Serial.println("Post AppendInfo");
    StreamBlob(Serial, buf, bufSize, 1);

    // hack in the has-been-repeated bit on the final address,
    // as was seen in the example on the website
    buf[13] |= 0b10000000;
    
    uint8_t bytesUsed = msg.Finalize();
    @fix@Serial.print("Post Finalize (");
    @fix@Serial.print(bytesUsed);
    @fix@Serial.print(" bytes used)");
    @fix@Serial.println();

    @fix@Serial.println("Container buffer");
    StreamBlob(Serial, buf, bufSize, 1);

    @fix@Serial.println("Completed buffer (just the used parts)");
    StreamBlob(Serial, buf, bytesUsed, 1);

    
    

    @fix@Serial.println();
    @fix@Serial.println();
}
 
void loop()
{

}












