#include "Evm.h"
#include "RFSI4463PRO.h"
#include "ModemBell202.h"
#include "AX25UIMessage.h"
#include "Str.h"
#include "Utl@fix@Serial.h"
#include "UtlStreamBlob.h"



Evm::Instance<10,10,10> evm;

static const uint8_t NUM_COMMANDS = 20;
static SerialShell<NUM_COMMANDS> shell;


static const uint8_t PIN_SS       = 25;
static const uint8_t PIN_SHUTDOWN = 14;

static RFSI4463PRO rf(PIN_SS, PIN_SHUTDOWN);


static const uint8_t BUF_SIZE = 60;
static uint8_t bufShared[BUF_SIZE] = { 0 };

static AX25UIMessage msg;
static ModemBell202Pwm modem;
static ModemAnalogPwm &ma = modem.GetModemAnalog();





void Send(uint8_t *buf, uint8_t bufLen)
{
    // APRSDroid sends ~256 flags to start...
    uint8_t flagList[]  = { 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E };
    uint8_t flagListLen = sizeof(flagList);

    uint8_t bitStuff = 0;

    rf.Start();
    modem.Start();

    // Send preamble, which also will serve as the flag byte
    // Send lots, like APRSDroid
    for (uint8_t i = 0; i < 6; ++i)
    {
        bitStuff = 0;
        modem.Send(flagList, flagListLen, bitStuff);
    }

    // send content
    bitStuff = 1;
    modem.Send(buf, bufLen, bitStuff);

    // Send trailing flags
    bitStuff = 0;
    modem.Send(flagList, 3, bitStuff);

    modem.Stop();
    rf.Stop();
}

void DoMessageTest()
{
    uint8_t *buf     = bufShared;
    uint8_t  bufSize = BUF_SIZE;
    
    msg.Init(buf, bufSize);

    msg.SetDstAddress("DST", 2);
    msg.SetSrcAddress("SRC", 3);
    msg.AddRepeaterAddress("WIDE1", 1);

    const char *info = "Just Some Data";
    uint8_t infoLen = strlen(info);   // 16
    msg.AppendInfo((uint8_t *)info, infoLen);
    
    uint8_t bytesUsed = msg.Finalize();

    @fix@Serial.println("Sending");

    Send(buf, bytesUsed);
}





void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");
    @fix@Serial.println();


    // Set up classes

    rf.Init();
    const uint32_t FREQUENCY_APRS = 144390000;
    rf.SetFrequency(FREQUENCY_APRS);
    
    modem.Init();

    
    // Set up interactive commands


    /////////////////////////////////////////////////////////
    //
    // RFSI4463PRO control
    //
    /////////////////////////////////////////////////////////
    
    auto rfstart = [](char *) {
        rf.Start();
    };
    shell.RegisterCommand("rfstart", rfstart);


    auto rfstop = [](char *) {
        rf.Stop();
    };
    shell.RegisterCommand("rfstop", rfstop);


    auto rffreq = [](char *cmdStr) {
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t freq = atol(str.TokenAtIdx(1, ' '));

            @fix@Serial.print(F("Setting freq to ")); @fix@Serial.println(freq);

            rf.SetFrequency(freq, 1);
        }
    };
    shell.RegisterCommand("rffreq", rffreq);


    auto rfdev = [](char *cmdStr) {
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t dev = atol(str.TokenAtIdx(1, ' '));

            @fix@Serial.print(F("Setting dev to ")); @fix@Serial.println(dev);

            rf.SetDeviation(dev, 1);
        }
    };
    shell.RegisterCommand("rfdev", rfdev);



    /////////////////////////////////////////////////////////
    //
    // ModemBell202 control
    //
    /////////////////////////////////////////////////////////

    auto mbstart = [](char *) {
        modem.Start();
    };
    shell.RegisterCommand("mbstart", mbstart);


    auto mbstop = [](char *) {
        modem.Stop();
    };
    shell.RegisterCommand("mbstop", mbstop);



    /////////////////////////////////////////////////////////
    //
    // ModemAnalog PWM control (underlying modem, any freq)
    //
    /////////////////////////////////////////////////////////

    auto mastart = [](char *) {
        ma.Start();
    };
    shell.RegisterCommand("mastart", mastart);

    auto mafreq = [](char *cmdStr) {
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t freq = atol(str.TokenAtIdx(1, ' '));

            @fix@Serial.print(F("Setting freq to ")); @fix@Serial.println(freq);

            ma.SetFrequency(freq);
        }
    };
    shell.RegisterCommand("mafreq", mafreq);

    auto mastop = [](char *) {
        ma.Stop();
    };
    shell.RegisterCommand("mastop", mastop);


    
    /////////////////////////////////////////////////////////
    //
    // Misc
    //
    /////////////////////////////////////////////////////////
    
    auto modpin = [](char *cmdStr) {
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            // this is the OC2B pin
            const uint8_t MODULATION_PIN = 5;
            
            uint8_t highLow = atoi(str.TokenAtIdx(1, ' '));

            @fix@Serial.print(F("Setting mod pin ")); @fix@Serial.println(highLow ? F("HIGH") : F("LOW"));

            PAL.PinMode(MODULATION_PIN, OUTPUT);
            PAL.DigitalWrite(MODULATION_PIN, highLow ? HIGH : LOW);
        }
    };
    shell.RegisterCommand("modpin", modpin);



    auto send = [](char *) {
        DoMessageTest();
    };
    shell.RegisterCommand("send", send);

    


    shell.RegisterErrorHandler([](char *cmdStr) {
        @fix@Serial.print("ERR: Unrecognized \"");
        @fix@Serial.print(cmdStr);
        @fix@Serial.print("\"");
        @fix@Serial.println();
    });


    // We know we want the modulation pin in a known state.
    // Let's pick low.  Why not.
    modpin((char *)"modpin 0");
    @fix@Serial.println();
    
    
    while (1)
    {
        shell.Run();
    }
}

void loop() {}

















