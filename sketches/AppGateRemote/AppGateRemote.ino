#include "PAL.h"
#include "Log.h"
#include "SerialInput.h"
#include "Evm.h"
#include "PinInput.h"
#include "RFSI4463PRO.h"




/*

10 bits

high
----
start: 1.347940
end  : 1.349390
diff: 0.00145 = 1.450ms

gap
---
next start: 1.349860
diff: 0.00047 = .470ms

low
---
start: 1.353710
end  : 1.354195
diff: 0.000485 = 0.485ms

gap
---
next start: 1.355640
diff: 0.001445 = 1.445ms


high duration plus gap: 1.450ms +  .470ms = 1.92ms
low  duration plus gap:  .485ms + 1.445ms = 1.93ms

total duration should then be: 10 x 1.92ms = 19.2ms (confirmed)


20ms delay between codes on the real remote.

 */



static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<20> shell;
static TimedEventHandlerDelegate ted;

static const uint8_t PIN_SS       = 25;
static const uint8_t PIN_SHUTDOWN = 14;
static const uint8_t PIN_OOK      =  5;

static RFSI4463PRO rf(PIN_SS, PIN_SHUTDOWN);

// drive this pin low first, otherwise it floats and accidentally
// gives input to the RF module and you see transmissions
static Pin pinOOK(PIN_OOK, LOW);

void SendCombination(uint16_t combo10bit)
{
    uint16_t bitShiftBuf = combo10bit;
    uint8_t  bitSentCount = 0;

    while (bitSentCount < 10)
    {
        uint8_t onOrOff = bitShiftBuf & 0x0001;
        bitShiftBuf >>= 1;

        if (onOrOff)
        {
            PAL.DigitalWrite(pinOOK, HIGH);
            PAL.DelayMicroseconds(1450);
            PAL.DigitalWrite(pinOOK, LOW);
            PAL.DelayMicroseconds(470);
        }
        else
        {
            PAL.DigitalWrite(pinOOK, HIGH);
            PAL.DelayMicroseconds(485);
            PAL.DigitalWrite(pinOOK, LOW);
            PAL.DelayMicroseconds(1445);
        }

        ++bitSentCount;
    }
}

static uint32_t betw = 20;

void SendCombinationMulti(uint16_t combo10bit, uint16_t count)
{
    for (uint8_t i = 0; i < count; ++i)
    {
        SendCombination(combo10bit);
        PAL.Delay(betw);
    }
}

void setup()
{
    LogStart(9600);
    Log("Starting");

    rf.Init();
    const uint32_t FREQ = 310000000;
    rf.SetFrequency(FREQ);
    rf.SetModulationType(RFSI4463PRO::ModulationType::MT_OOK);
    rf.Start();

    const uint16_t COMBO = 0b0000001110000111;


    static const uint32_t BIT_PATTERN_WARMUP = 0b10101010101010;

    static uint8_t warmSendCount = 3;
    static uint8_t codeSendCount = 1;
    static uint8_t postSendCount = 0;

    shell.RegisterCommand("warm", [&](char *cmdStr){
        Str str(cmdStr);
  
        if (str.TokenCount(' ') == 2)
        {
            warmSendCount = atoi(str.TokenAtIdx(1, ' '));

            Log("WarmSendCount = ", warmSendCount);
        }
    });

    shell.RegisterCommand("code", [&](char *cmdStr){
        Str str(cmdStr);
  
        if (str.TokenCount(' ') == 2)
        {
            codeSendCount = atoi(str.TokenAtIdx(1, ' '));

            Log("CodeSendCount = ", codeSendCount);
        }
    });
    
    shell.RegisterCommand("post", [&](char *cmdStr){
        Str str(cmdStr);
  
        if (str.TokenCount(' ') == 2)
        {
            postSendCount = atoi(str.TokenAtIdx(1, ' '));

            Log("postSendCount = ", postSendCount);
        }
    });

    shell.RegisterCommand("betw", [&](char *cmdStr){
        Str str(cmdStr);
  
        if (str.TokenCount(' ') == 2)
        {
            betw = atoi(str.TokenAtIdx(1, ' '));

            Log("betw = ", betw);
        }
    });

    // Send
    shell.RegisterCommand("send", [](char *){
        // Warm up receiver
        SendCombinationMulti(BIT_PATTERN_WARMUP, warmSendCount);

        // Send code
        SendCombinationMulti(COMBO, codeSendCount);
        
        // Send outro
        SendCombinationMulti(BIT_PATTERN_WARMUP, postSendCount);
    });

    shell.Start();


    
    Log("Running");
    evm.MainLoop();
}



void loop() { }












