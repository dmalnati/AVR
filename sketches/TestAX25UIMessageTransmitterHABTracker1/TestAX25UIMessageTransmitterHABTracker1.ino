#include "AX25UIMessageTransmitter.h"
#include "APRSPositionReportMessageHABTracker1.h"


static uint8_t PIN_TX_ENABLE = 14;
static AX25UIMessageTransmitter<> amt(PIN_TX_ENABLE);


void Test()
{
    AX25UIMessage &msg = *amt.GetAX25UIMessage();


    msg.SetDstAddress("APZ001", 0);
    msg.SetSrcAddress("KD2KDD", 9);
    msg.AddRepeaterAddress("WIDE1", 1);

    // Add APRS data
    uint8_t *bufInfo    = NULL;
    uint8_t  bufInfoLen = 0;

    APRSPositionReportMessageHABTracker1 aprm;

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


    // Configure and Transmit
    amt.SetFlagStartDurationMs(300);
    amt.SetFlagEndDurationMs(10);
    amt.SetTransmitCount(2);
    amt.SetDelayMsBetweenTransmits(2000);

    amt.Transmit();
}

void setup()
{
    amt.Init();


    while (1)
    {
        Test();

        PAL.Delay(2000);
    }
}

void loop() {}
