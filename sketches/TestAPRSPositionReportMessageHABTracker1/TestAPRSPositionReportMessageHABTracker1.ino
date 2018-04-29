#include "PAL.h"
#include "UtlStreamBlob.h"
#include "APRSPositionReportMessageHABTracker1.h"


static const uint16_t BUF_SIZE = 128;
static char buf[BUF_SIZE + 1] = { '\0' };

static APRSPositionReportMessageHABTracker1 aprm;


void BuildMessageBase()
{
    aprm.SetTargetBuf((uint8_t *)buf, BUF_SIZE);

    // Mandatory fields
    aprm.SetTimeLocal(19, 14, 7);
    aprm.SetLatitude(40, 44, 13.87);
    aprm.SetSymbolTableID('/');
    aprm.SetLongitude(-74, 2, 2.32);
    aprm.SetSymbolCode('O');

    // Extended standard
    aprm.SetCommentCourseAndSpeed(273, 777);
    aprm.SetCommentAltitude(444);
}

void BuildMessageAscii()
{
    BuildMessageBase();
    
    // My own
    aprm.SetCommentBarometricPressure(10132);   // sea level
    aprm.SetCommentTemperature(72); // first thermometer, inside(?)
    aprm.SetCommentMagnetics(-0.2051, 0.0527, 0.0742);    // on my desk
    aprm.SetCommentAcceleration(56.7017, 1042.7856, -946.2891);    // on my desk, modified y
    aprm.SetCommentTemperature(74); // the other thermometer, outside(?)
    aprm.SetCommentVoltage(4.723);

    
    @fix@Serial.print("Bytes (ASCII Msg): ");
    @fix@Serial.print(aprm.GetBytesUsed());
    @fix@Serial.println();
    @fix@Serial.println(buf);
    @fix@Serial.println();

    StreamBlob(Serial, (uint8_t *)buf, BUF_SIZE, 1, 1);

    @fix@Serial.println();
    @fix@Serial.println();
    @fix@Serial.println();
    @fix@Serial.println();
}

void BuildMessageBinaryEncoded()
{
    BuildMessageBase();
    
    // My own
    aprm.SetCommentBarometricPressureBinaryEncoded(10132);   // sea level
    aprm.SetCommentTemperatureBinaryEncoded(72); // first thermometer, inside(?)
    aprm.SetCommentMagneticsBinaryEncoded(-0.2051, 0.0527, 0.0742);    // on my desk
    aprm.SetCommentAccelerationBinaryEncoded(56.7017, 1042.7856, -946.2891);    // on my desk, modified y
    aprm.SetCommentTemperatureBinaryEncoded(74); // the other thermometer, outside(?)
    aprm.SetCommentVoltageBinaryEncoded(4.723);
    aprm.SetCommentSeqNoBinaryEncoded(15);

    
    @fix@Serial.print("Bytes (Binary Msg): ");
    @fix@Serial.print(aprm.GetBytesUsed());
    @fix@Serial.println();
    @fix@Serial.println(buf);
    @fix@Serial.println();

    StreamBlob(Serial, (uint8_t *)buf, BUF_SIZE, 1, 1);

    @fix@Serial.println();
    @fix@Serial.println();
    @fix@Serial.println();
    @fix@Serial.println();
}


void setup()
{
    @fix@Serial.begin(9600);

    while (1)
    {
        BuildMessageAscii();
        BuildMessageBinaryEncoded();
        
        PAL.Delay(1000);
    }
}



void loop() { }












