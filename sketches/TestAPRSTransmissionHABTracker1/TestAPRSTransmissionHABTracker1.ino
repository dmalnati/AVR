#include "PAL.h"
#include "UtlStreamBlob.h"
#include "AX25UIMessage.h"
#include "APRSPositionReportMessageHABTracker1.h"
#include "ModemBell202.h"


static const uint8_t BUF_SIZE = 200;
static uint8_t bufShared[BUF_SIZE] = { 0 };

static AX25UIMessage msg;
static APRSPositionReportMessageHABTracker1 aprm;
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

    @fix@Serial.println("SetupMessage");
    
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
        aprm.SetSymbolCode('O');
        
        // extended
        aprm.SetCommentCourseAndSpeed(273, 777);
        aprm.SetCommentAltitude(444);

        // my extensions
        aprm.SetCommentBarometricPressureBinaryEncoded(10132);   // sea level
        aprm.SetCommentTemperatureBinaryEncoded(72); // first thermometer, inside(?)
        aprm.SetCommentMagneticsBinaryEncoded(-0.2051, 0.0527, 0.0742);    // on my desk
        aprm.SetCommentAccelerationBinaryEncoded(56.7017, 1042.7856, -946.2891);    // on my desk, modified y
        aprm.SetCommentTemperatureBinaryEncoded(74); // the other thermometer, outside(?)
        aprm.SetCommentVoltageBinaryEncoded(4.723);

        static uint16_t seqNo = 0;
        aprm.SetCommentSeqNoBinaryEncoded(++seqNo);

        msg.AssertInfoBytesUsed(aprm.GetBytesUsed());
    }

    uint8_t bytesUsed = msg.Finalize();

    @fix@Serial.print("Bytes used: ");
    @fix@Serial.print(bytesUsed);
    @fix@Serial.println();
    StreamBlob(Serial, buf, bytesUsed, 1, 1);
    @fix@Serial.println();

    Send(buf, bytesUsed);
}



void setup()
{
    @fix@Serial.begin(9600);

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












