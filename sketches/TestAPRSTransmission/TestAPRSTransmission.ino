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

    // So I can use Serial in the class for debugging...
    ModemBell202 modemReal;
    modem = &modemReal;
    
    uint8_t *buf;
    uint8_t  bufLen;

    //TestModemInLoopOnDelay(500);
    
    //TestModemRandByteOnDelay(50);

    GetSetUpBuffer(&buf, &bufLen);
    //GetSetUpTestBuffer(&buf, &bufLen);
    //GetSetUpTestBuffer2(&buf, &bufLen);
    //GetSetUpTestBufferMatchAPRSDroid(&buf, &bufLen);
    SendDataInLoopOnDelay(buf, bufLen, 1000);
}

void DPrint(const char *msg)
{
    return;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        Serial.println(msg);
    }
}

void TestModemRandByteOnDelay(uint16_t delayMs)
{
    srand(analogRead(15));

    const uint8_t VAL_LIST_LEN = 5;
    uint8_t valList[VAL_LIST_LEN];

    uint8_t bitStuff = 0;

    while (1)
    {
        // Generate some new numbers
        for (uint8_t i = 0; i < VAL_LIST_LEN; ++i)
        {
            // range between 0 - 255 inclusive
            valList[i] = rand() % 256;
        }

        // Spit out ever increasing numbers of them to make
        // measuring byte durations easier
        for (uint8_t i = 1; i <= VAL_LIST_LEN; ++i)
        {
            for (uint8_t j = 0; j < i; ++j)
            {
                modem->Start();
        
                bitStuff = 0;
                modem->Send(valList, j + 1, bitStuff);
        
                modem->Stop();

                PAL.Delay(delayMs);
            }
        }

        PAL.Delay(delayMs);
    }
}

void TestModemInLoopOnDelay(uint16_t delayMs)
{
    uint8_t buf[]  = { 0x7E, 0x7E };
    uint8_t bufLen = sizeof(buf);

    uint8_t subQuantity = 1;
    uint8_t bitStuff = 0;

    while (1)
    {
        // Test alternating 1 or 2 byte sends
        modem->Start();

        bitStuff = 0;
        modem->Send(buf, bufLen - subQuantity, bitStuff);

        modem->Stop();

        subQuantity = !subQuantity;


        // Wait a moment
        PAL.Delay(delayMs);


        // Test just to see how well two consecutive Send() calls work
        modem->Start();

        bitStuff = 0;
        modem->Send(buf, bufLen, bitStuff);
        modem->Send(buf, bufLen, bitStuff);

        modem->Stop();


        // Wait a moment
        PAL.Delay(delayMs);
    }
}

void SendDataInLoopOnDelay(uint8_t *buf, uint8_t bufLen, uint16_t delayMs)
{
    // APRSDroid sends ~256 flags to start...
    uint8_t flagList[]  = { 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E };
    uint8_t flagListLen = sizeof(flagList);

    uint8_t zeroList[]  = { 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t zeroListLen = sizeof(zeroList);

    uint8_t bitStuff = 0;

    while (1)
    {
        DPrint("Before Start");
        
        modem->Start();

        DPrint("Before Zeros");

        // Send some zeros, which cause lots of bit transitions, good
        // for syncing (according to the internet)
        for (uint8_t i = 0; i < 3; ++i)
        {
            bitStuff = 0;
            modem->Send(zeroList, zeroListLen, bitStuff);
        }

        DPrint("Before Flags");
        
        // Send preamble, which also will serve as the flag byte
        // Send lots, like APRSDroid
        for (uint8_t i = 0; i < 20; ++i)
        {
            bitStuff = 0;
            modem->Send(flagList, flagListLen, bitStuff);
        }

        // Debug, stuff SOH in there since APRSDroid is doing it??
        bitStuff = 1;
        uint8_t soh = 0x01 << 1;
        modem->Send(&soh, 1, bitStuff);

        DPrint("Before Data");

        // send content
        bitStuff = 1;
        modem->Send(buf, bufLen, bitStuff);

        DPrint("Before Trailer");

        // Send trailing flags
        bitStuff = 0;
        modem->Send(flagList, 2, bitStuff);

        DPrint("Before Stop");

        modem->Stop();

        DPrint("Before Delay");

        PAL.Delay(delayMs);
    }
}

void GetSetUpBuffer(uint8_t **bufRet, uint8_t *bufLenRet)
{
    uint8_t *buf     = bufShared;
    
    msg.Init(buf);

    const char *addrDst     = "KD2KDD";
    uint8_t     addrDstSSID = 9;
    const char *addrSrc     = "ABC123";
    uint8_t     addrSrcSSID = 7;
    const char *info1       = "hi ";
    const char *info2       = "mom!";

    msg.SetDstAddress(addrDst, addrDstSSID);
    msg.SetDstAddress(addrSrc, addrSrcSSID);
    
    msg.AppendInfo((uint8_t *)info1, strlen(info1));
    msg.AppendInfo((uint8_t *)info2, strlen(info2));
    
    uint8_t bytesUsed = msg.Finalize();

    Serial.println("Completed buffer (just the used parts)");
    StreamBlob(Serial, buf, bytesUsed, 1);
    Serial.println();

    // Fill out return parameters
    *bufRet    = buf;
    *bufLenRet = bytesUsed;
}

// https://www.tapr.org/pdf/DCC1998-PICet-W2FS.pdf
void GetSetUpTestBuffer(uint8_t **bufRet, uint8_t *bufLenRet)
{
    uint8_t *buf = bufShared;
    
    const char *addrDst          = "CQ";
    uint8_t     addrDstSSID      = 0;
    const char *addrSrc          = "W2FS";
    uint8_t     addrSrcSSID      = 4;
    const char *addrRepeater     = "RELAY";
    uint8_t     addrRepeaterSSID = 0;
    const char *info             = "Test";
    
    msg.Init(buf);
    msg.SetDstAddress(addrDst, addrDstSSID);
    msg.SetDstAddress(addrSrc, addrSrcSSID);
    msg.AddRepeaterAddress(addrRepeater, addrRepeaterSSID);
    msg.AppendInfo((uint8_t *)info, strlen(info));
    uint8_t bytesUsed = msg.Finalize();
    
    Serial.println("Completed buffer (just the used parts)");
    StreamBlob(Serial, buf, bytesUsed, 1);
    Serial.println();

    // Fill out return parameters
    *bufRet    = buf;
    *bufLenRet = bytesUsed;
}



// http://practicingelectronics.com/articles/article-100003/article.php
void GetSetUpTestBuffer2(uint8_t **bufRet, uint8_t *bufLenRet)
{
    uint8_t *buf = bufShared;
    
    const char *addrDst          = "CQ";
    uint8_t     addrDstSSID      = 0;
    const char *addrSrc          = "KB2BRD";
    uint8_t     addrSrcSSID      = 2;
    const char  info[]           = { 'A', '\r' };
    
    msg.Init(buf);
    msg.SetDstAddress(addrDst, addrDstSSID);
    msg.SetDstAddress(addrSrc, addrSrcSSID);
    msg.AppendInfo((uint8_t *)info, sizeof(info));
    uint8_t bytesUsed = msg.Finalize();
    
    Serial.println("Completed buffer (just the used parts)");
    StreamBlob(Serial, buf, bytesUsed, 1);
    Serial.println();

    // Fill out return parameters
    *bufRet    = buf;
    *bufLenRet = bytesUsed;
}

void GetSetUpTestBufferMatchAPRSDroid(uint8_t **bufRet, uint8_t *bufLenRet)
{
    uint8_t *buf = bufShared;
    
    const char *addrDst          = "CQ";
    uint8_t     addrDstSSID      = 0;
    const char *addrSrc          = "KD2KDD";
    uint8_t     addrSrcSSID      = 9;
    const char *addrRepeater     = "WIDE1";
    uint8_t     addrRepeaterSSID = 1;
    const char *info             = "1234";
    
    msg.Init(buf);
    msg.SetDstAddress(addrDst, addrDstSSID);
    msg.SetDstAddress(addrSrc, addrSrcSSID);
    msg.AddRepeaterAddress(addrRepeater, addrRepeaterSSID);
    msg.AppendInfo((uint8_t *)info, strlen(info));
    uint8_t bytesUsed = msg.Finalize();
    
    Serial.println("Completed buffer (just the used parts)");
    StreamBlob(Serial, buf, bytesUsed, 1);
    Serial.println();

    // Fill out return parameters
    *bufRet    = buf;
    *bufLenRet = bytesUsed;
}


void loop() { }












