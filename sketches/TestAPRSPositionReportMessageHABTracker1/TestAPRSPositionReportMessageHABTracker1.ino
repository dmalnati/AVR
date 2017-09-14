#include "PAL.h"
#include "UtlStreamBlob.h"
#include "APRSPositionReportMessageHABTracker1.h"


static const uint16_t BUF_SIZE = 128;

static APRSPositionReportMessageHABTracker1 aprm;


void BuildMessage()
{
    char buf[BUF_SIZE + 1];

    buf[BUF_SIZE] = '\0';

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

    // My own
    aprm.SetCommentBarometricPressure(10132);   // sea level
    aprm.SetCommentTemperature(72); // first thermometer, inside(?)
    aprm.SetCommentMagnetics(-0.2051, 0.0527, 0.0742);    // on my desk
    aprm.SetCommentAcceleration(56.7017, 42.7856, -946.2891);    // on my desk
    aprm.SetCommentTemperature(74); // the other thermometer, outside(?)
    aprm.SetCommentVoltage(4.723);

    
    Serial.print("Bytes: ");
    Serial.print(aprm.GetBytesUsed());
    Serial.println();
    Serial.println(buf);
    Serial.println();

    StreamBlob(Serial, (uint8_t *)buf, BUF_SIZE, 1, 1);

    Serial.println();
    Serial.println();
    Serial.println();
    Serial.println();
}


void setup()
{
    Serial.begin(9600);

    while (1)
    {
        BuildMessage();
        
        PAL.Delay(1000);
    }
}



void loop() { }












