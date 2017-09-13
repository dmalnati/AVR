#include "PAL.h"
#include "UtlStreamBlob.h"
#include "APRSPositionReportMessage.h"


static const uint16_t BUF_SIZE = 80;

static APRSPositionReportMessage aprm;


void BuildMessage()
{
    char buf[BUF_SIZE + 1];

    buf[BUF_SIZE] = '\0';

    aprm.SetTargetBuf((uint8_t *)buf, BUF_SIZE);

    aprm.SetTimeLocal(19, 14, 7);
    aprm.SetLatitude(40, 44, 13.87);
    aprm.SetSymbolTableID('/');
    aprm.SetLongitude(-74, 2, 2.32);
    aprm.SetSymbolCode('O');
    
    
    Serial.println("Before Comment appendages");
    Serial.print("bytesUsed: ");
    Serial.print(aprm.GetBytesUsed());
    Serial.println();
    Serial.println(buf);
    Serial.println();

    aprm.SetCommentCourseAndSpeed(273, 777);
    Serial.println("After Comment Course/Speed");
    Serial.print("bytesUsed: ");
    Serial.print(aprm.GetBytesUsed());
    Serial.println();
    Serial.println(buf);
    Serial.println();

    aprm.SetCommentAltitude(444);
    Serial.println("After Comment Altitude");
    Serial.print("bytesUsed: ");
    Serial.print(aprm.GetBytesUsed());
    Serial.println();
    Serial.println(buf);
    Serial.println();

    aprm.AppendCommentString("hi mom!");
    Serial.println("After Comment string appendage");
    Serial.print("bytesUsed: ");
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












