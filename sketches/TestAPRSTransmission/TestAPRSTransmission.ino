#define IVM_DISABLE

#include "PAL.h"
#include "UtlStreamBlob.h"
#include "AX25UIMessage.h"
#include "ModemBell202.h"



static const uint8_t BUF_SIZE = 60;
static uint8_t bufShared[BUF_SIZE] = { 0 };


static AX25UIMessage msg;
static ModemBell202 *modem;


void setup()
{
    Serial.begin(9600);

    ModemBell202 modemReal;
    modem = &modemReal;
    
    uint8_t *buf;
    uint8_t  bufLen;

    //TestModemInLoopOnDelay(500);

    GetSetUpBuffer(&buf, &bufLen);
    SendDataInLoopOnDelay(buf, bufLen, 1000);
}

void TestModemInLoopOnDelay(uint16_t delayMs)
{
    uint8_t buf[]  = { 0x7E, 0x7E };
    uint8_t bufLen = sizeof(buf);

    uint8_t subQuantity = 1;

    while (1)
    {
        modem->Start();

        uint8_t bitStuff = 0;
        uint8_t nrzi     = 0;
        modem->Send(buf, bufLen - subQuantity, bitStuff, nrzi);

        modem->Stop();

        subQuantity = !subQuantity;

        PAL.Delay(delayMs);
    }
}

void SendDataInLoopOnDelay(uint8_t *buf, uint8_t bufLen, uint16_t delayMs)
{
    uint8_t *bufNonDelim = &buf[1];
    uint8_t  bufNonDelimLen = bufLen - 2;

    // APRSDroid sends ~256 flags to start...
    uint8_t flagList[]  = { 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E };
    uint8_t flagListLen = sizeof(flagList);

    uint8_t bitStuff = 0;
    uint8_t nrzi     = 0;

    while (1)
    {
        modem->Start();
        
        // Send preamble, which also will serve as the flag byte
        // Send lots, like APRSDroid
        for (uint8_t i = 0; i < 10; ++i)
        {
            bitStuff = 0;
            modem->Send(flagList, flagListLen, bitStuff, nrzi);
        }

        // send content
        bitStuff = 1;
        nrzi     = 1;
        modem->Send(bufNonDelim, bufNonDelimLen, bitStuff, nrzi);

        // Send trailing flags
        bitStuff = 0;
        nrzi     = 0;
        modem->Send(flagList, flagListLen, bitStuff, nrzi);

        modem->Stop();

        PAL.Delay(delayMs);
    }
}

void GetSetUpBuffer(uint8_t **bufRet, uint8_t *bufLenRet)
{
    uint8_t *buf     = bufShared;
    uint8_t  bufSize = BUF_SIZE;
    
    msg.Init(buf);

    const char *addrDst7char    = "KD2KDD0";
    const char *addrSrc7char    = "DK3DKK9";
    const char *addrRepeaterStr = NULL;

    msg.SetAddress(addrDst7char, addrSrc7char, addrRepeaterStr);

    //Serial.println("Post SetAddress");
    //StreamBlob(Serial, buf, bufSize, 1);


    const char *info1 = "hi ";
    const char *info2 = "mom!";
    
    msg.AppendInfo((uint8_t *)info1, strlen(info1));

    //Serial.println("Post AppendInfo1");
    //StreamBlob(Serial, buf, bufSize, 1);
    
    msg.AppendInfo((uint8_t *)info2, strlen(info2));
    
    //Serial.println("Post AppendInfo2");
    //StreamBlob(Serial, buf, bufSize, 1);


    uint8_t bytesUsed = msg.Finalize();
    //Serial.print("Post Finalize (");
    //Serial.print(bytesUsed);
    //Serial.print(" bytes used)");
    //Serial.println();

    //Serial.println("Container buffer");
    //StreamBlob(Serial, buf, bufSize, 1);

    //Serial.println("Completed buffer (just the used parts)");
    //StreamBlob(Serial, buf, bytesUsed, 1);

    // Fill out return parameters
    *bufRet    = buf;
    *bufLenRet = bytesUsed;
}
 
void loop() { }












