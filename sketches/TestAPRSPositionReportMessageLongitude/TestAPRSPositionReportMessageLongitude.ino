#include "PAL.h"
#include "Evm.h"
#include "UtlSerial.h"
#include "APRSPositionReportMessage.h"


static Evm::Instance<10,10,10>         evm;
static SerialAsyncConsoleEnhanced<10>  console;


void TestLongitude(int16_t degrees, uint8_t minutes, double seconds)
{
    const uint8_t BUF_SIZE = 100;
    char buf[BUF_SIZE];

    memset(buf, ' ', BUF_SIZE);

    APRSPositionReportMessage aprm;
    
    aprm.SetTargetBuf((uint8_t *)buf, BUF_SIZE);

    aprm.SetLongitude(degrees, minutes, seconds);

    Serial.print("Buf: \"");
    Serial.write(buf, BUF_SIZE);
    Serial.print("\"");
    Serial.println();
    
    Serial.print("Bytes: "); Serial.println(aprm.GetBytesUsed());
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    console.RegisterCommand("test", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 4)
        {
            int16_t degrees = atol(str.TokenAtIdx(1, ' '));
            uint8_t minutes = atoi(str.TokenAtIdx(2, ' '));
            double  seconds = atof(str.TokenAtIdx(3, ' '));

            Serial.print(F("Testing: "));
            Serial.print(degrees);
            Serial.print(" ");
            Serial.print(minutes);
            Serial.print(" ");
            Serial.print(seconds);
            Serial.println();

            TestLongitude(degrees, minutes, seconds);
        }
    });

    console.Start();

    evm.MainLoop();
}

void loop() {}



