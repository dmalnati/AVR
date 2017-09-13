#include "PAL.h"
#include "UtlStreamBlob.h"
#include "AX25UIMessage.h"
#include "APRSPositionReportMessage.h"
#include "ModemBell202.h"


static const uint8_t BUF_SIZE = 200;
static uint8_t bufShared[BUF_SIZE] = { 0 };

static AX25UIMessage msg;
static APRSPositionReportMessage aprm;
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
    for (uint8_t i = 0; i < 6; ++i)
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



void SetupMessage()
{
    uint8_t *buf = bufShared;

    Serial.println("SetupMessage");
    
    msg.Init(buf, BUF_SIZE);

    msg.SetDstAddress("APZ001", 0);
    msg.SetSrcAddress("KD2KDD", 9);
    msg.AddRepeaterAddress("WIDE1", 1);

    // Add APRS data
    uint8_t *bufInfo    = NULL;
    uint8_t  bufInfoLen = 0;

    if (msg.GetUnsafePtrInfo(&bufInfo, &bufInfoLen))
    {
        aprm.SetTargetBuf(bufInfo, bufInfoLen);
    
        aprm.SetTimeLocal(19, 14, 7);
        aprm.SetLatitude(40, 44, 13.87);
        aprm.SetSymbolTableID('/');
        aprm.SetLongitude(-74, 2, 2.32);
        aprm.SetLongitude(-74, 2, 59.99);
        aprm.SetSymbolCode('O');
        aprm.SetCommentCourseAndSpeed(273, 777);
        aprm.SetCommentAltitude(444);
        aprm.AppendCommentString("hi mom!");

        msg.AssertInfoBytesUsed(aprm.GetBytesUsed());
    }

    uint8_t bytesUsed = msg.Finalize();

    Serial.print("Bytes used: ");
    Serial.print(bytesUsed);
    Serial.println();
    StreamBlob(Serial, buf, bytesUsed, 1, 1);
    Serial.println();

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
        SetupMessage();
        PAL.Delay(1000);
    }
}



void loop() { }












