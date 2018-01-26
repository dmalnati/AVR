#include "RFSI4463PRO.h"
#include "AX25UIMessageTransmitter.h"
#include "APRSPositionReportMessageHABTracker1.h"


static const uint8_t PIN_SS       = 25;
static const uint8_t PIN_SHUTDOWN = 14;

static RFSI4463PRO radio(PIN_SS, PIN_SHUTDOWN);
static AX25UIMessageTransmitter<> amt;


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
    amt.SetTransmitCount(1);
    amt.SetDelayMsBetweenTransmits(2000);

    static uint32_t timeLast = 0;
    Serial.print("Transmitting - ");
    uint32_t timeNow = PAL.Millis();
    uint32_t timeDiff = timeNow - timeLast;
    timeLast = timeNow;
    Serial.print(timeDiff);
    Serial.println(" ms since last");
    
    amt.Transmit();
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");
    
    radio.SetFrequency(144390000);
    radio.Init();
    amt.Init([](){ radio.Start(); }, [](){ radio.Stop(); });


    while (1)
    {
        Test();

        PAL.Delay(5000);
    }
}

void loop() {}
