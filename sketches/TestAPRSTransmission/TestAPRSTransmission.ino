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
    SendDataInLoopOnDelay(buf, bufLen, 1000);
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

    uint8_t bitStuff = 0;

    while (1)
    {
        #if 0
        Serial.println("TOP");

        Serial.print("1 - TCCR1B: ");
        Serial.println(TCCR1B);
        Serial.print("1 - TCCR2B: ");
        Serial.println(TCCR2B);
        #endif
        
        modem->Start();

        #if 0
        Serial.print("2 - TCCR1B: ");
        Serial.println(TCCR1B);
        Serial.print("2 - TCCR2B: ");
        Serial.println(TCCR2B);
        #endif

        
        
        // Send preamble, which also will serve as the flag byte
        // Send lots, like APRSDroid
        for (uint8_t i = 0; i < 20; ++i)
        {
            bitStuff = 0;
            modem->Send(flagList, flagListLen, bitStuff);
        }

        // send content
        bitStuff = 1;
        modem->Send(buf, bufLen, bitStuff);

        // Send trailing flags
        bitStuff = 0;
        modem->Send(flagList, 2, bitStuff);

        modem->Stop();

        PAL.Delay(delayMs);

        #if 0
        Serial.println("BOTTOM");
        #endif
    }
}

void GetSetUpBuffer(uint8_t **bufRet, uint8_t *bufLenRet)
{
    uint8_t *buf     = bufShared;
    //uint8_t  bufSize = BUF_SIZE;
    
    msg.Init(buf);

    const char *addrDst     = "KD2KD";      // too short on purpose, checking padding logic
    uint8_t     addrDstSSID = 0;
    const char *addrSrc     = "DK3DKK9";    // too long on purpose, checking truncation logic
    uint8_t     addrSrcSSID = 1;

    msg.SetAddress(addrDst, addrDstSSID, addrSrc, addrSrcSSID);

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

    Serial.println("Completed buffer (just the used parts)");
    StreamBlob(Serial, buf, bytesUsed, 1);
    Serial.println();

    // Fill out return parameters
    *bufRet    = buf;
    *bufLenRet = bytesUsed;
}
 
void loop() { }












