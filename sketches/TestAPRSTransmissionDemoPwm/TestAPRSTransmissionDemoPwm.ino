#include "PAL.h"
#include "UtlStreamBlob.h"
#include "AX25UIMessage.h"
#include "ModemBell202.h"
#include "RFSI4463PRO.h"
#include "RFSI4463PRODebug.h"

static const uint8_t PIN_SS       = 25;
static const uint8_t PIN_SHUTDOWN = 14;

static RFSI4463PRO rf(PIN_SS, PIN_SHUTDOWN);




// A program that works, but doesn't use my callsign, so
// I can post concrete examples on the internet without worrying that
// other people will spam the airwaves with my info while debugging
// their systems.


static const uint8_t BUF_SIZE = 60;
static uint8_t bufShared[BUF_SIZE] = { 0 };

static AX25UIMessage msg;
static ModemBell202Pwm *modem;



void Send(uint8_t *buf, uint8_t bufLen)
{
    // APRSDroid sends ~256 flags to start...
    uint8_t flagList[]  = { 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E };
    uint8_t flagListLen = sizeof(flagList);

    uint8_t bitStuff = 0;

    rf.Start();
    {
        RFSI4463PRO::REQUEST_DEVICE_STATE_REP rep;
        rf.Command_REQUEST_DEVICE_STATE(rep);
        @fix@Serial.print("State: ");  @fix@Serial.println(rep.CURR_STATE.MAIN_STATE);
        @fix@Serial.print("Channel: ");  @fix@Serial.println(rep.CURRENT_CHANNEL.CURRENT_CHANNEL);
    }
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
    rf.Stop();
    {
        RFSI4463PRO::REQUEST_DEVICE_STATE_REP rep;
        rf.Command_REQUEST_DEVICE_STATE(rep);
        @fix@Serial.print("State: ");  @fix@Serial.println(rep.CURR_STATE.MAIN_STATE);
        @fix@Serial.print("Channel: ");  @fix@Serial.println(rep.CURRENT_CHANNEL.CURRENT_CHANNEL);
    }
}

void DoMessageTest()
{
    uint8_t *buf     = bufShared;
    uint8_t  bufSize = BUF_SIZE;
    
    msg.Init(buf, bufSize);

    msg.SetDstAddress("DST", 2);
    msg.SetSrcAddress("SRC", 3);
    msg.AddRepeaterAddress("WIDE1", 1);

    //@fix@Serial.println("Post SetAddress");
    //StreamBlob(Serial, buf, bufSize, 1);

    const char *info = "Just Some Data";
    uint8_t infoLen = strlen(info);   // 16
    msg.AppendInfo((uint8_t *)info, infoLen);

    //@fix@Serial.println("Post AppendInfo");
    //StreamBlob(Serial, buf, bufSize, 1);
    
    uint8_t bytesUsed = msg.Finalize();
    //@fix@Serial.print("Post Finalize (");
    //@fix@Serial.print(bytesUsed);
    //@fix@Serial.print(" bytes used)");
    //@fix@Serial.println();

    //@fix@Serial.println("Container buffer");
    //StreamBlob(Serial, buf, bufSize, 1);

    //@fix@Serial.println("Completed buffer (just the used parts)");
    //StreamBlob(Serial, buf, bytesUsed, 1);

    @fix@Serial.println("Sending");
    @fix@Serial.println();
    @fix@Serial.println();

    Send(buf, bytesUsed);
}

void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    // So I can use Serial in the class for debugging...
    ModemBell202Pwm modemReal;
    modem = &modemReal;

    rf.Init();
    const uint32_t FREQUENCY_APRS = 144390000;
    rf.SetFrequency(FREQUENCY_APRS);
    
    modem->Init();

    while (1)
    {
        DoMessageTest();
        PAL.Delay(5000);
    }
}



void loop() { }












