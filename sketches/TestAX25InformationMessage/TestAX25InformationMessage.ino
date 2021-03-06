#define IVM_DISABLE

#include "PAL.h"
#include "UtlStreamBlob.h"
#include "AX25UIMessage.h"

static const uint8_t PIN = 15;

static const uint8_t BUF_SIZE = 60;
static uint8_t bufShared[BUF_SIZE] = { 0 };

static AX25UIMessage msg;


void setup()
{
    @fix@Serial.begin(9600);
    
    uint8_t *buf     = bufShared;
    uint8_t  bufSize = BUF_SIZE;
    
    msg.Init(buf);

    const char *addrDst7char    = "KD2KDD0";
    const char *addrSrc7char    = "DK3DKK9";
    const char *addrRepeaterStr = NULL;

    msg.SetAddress(addrDst7char, addrSrc7char, addrRepeaterStr);

    @fix@Serial.println("Post SetAddress");
    StreamBlob(Serial, buf, bufSize, 1);


    const char *info1 = "hi ";
    const char *info2 = "mom!";
    
    msg.AppendInfo((uint8_t *)info1, strlen(info1));

    @fix@Serial.println("Post AppendInfo1");
    StreamBlob(Serial, buf, bufSize, 1);
    
    msg.AppendInfo((uint8_t *)info2, strlen(info2));
    
    @fix@Serial.println("Post AppendInfo2");
    StreamBlob(Serial, buf, bufSize, 1);


    uint8_t bytesUsed = msg.Finalize();
    @fix@Serial.print("Post Finalize (");
    @fix@Serial.print(bytesUsed);
    @fix@Serial.print(" bytes used)");
    @fix@Serial.println();

    @fix@Serial.println("Container buffer");
    StreamBlob(Serial, buf, bufSize, 1);

    @fix@Serial.println("Completed buffer (just the used parts)");
    StreamBlob(Serial, buf, bytesUsed, 1);

    




    
}
 
void loop()
{

}












