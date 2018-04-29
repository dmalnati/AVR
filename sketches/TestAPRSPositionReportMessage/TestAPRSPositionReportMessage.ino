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
    
    
    @fix@Serial.println("Before Comment appendages");
    @fix@Serial.print("bytesUsed: ");
    @fix@Serial.print(aprm.GetBytesUsed());
    @fix@Serial.println();
    @fix@Serial.println(buf);
    @fix@Serial.println();

    aprm.SetCommentCourseAndSpeed(273, 777);
    @fix@Serial.println("After Comment Course/Speed");
    @fix@Serial.print("bytesUsed: ");
    @fix@Serial.print(aprm.GetBytesUsed());
    @fix@Serial.println();
    @fix@Serial.println(buf);
    @fix@Serial.println();

    aprm.SetCommentAltitude(444);
    @fix@Serial.println("After Comment Altitude");
    @fix@Serial.print("bytesUsed: ");
    @fix@Serial.print(aprm.GetBytesUsed());
    @fix@Serial.println();
    @fix@Serial.println(buf);
    @fix@Serial.println();

    aprm.AppendCommentString("hi mom!");
    @fix@Serial.println("After Comment string appendage");
    @fix@Serial.print("bytesUsed: ");
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
        BuildMessage();
        
        PAL.Delay(1000);
    }
}



void loop() { }












