#include "PAL.h"
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

 */



static const uint8_t PIN_SS       = 25;
static const uint8_t PIN_SHUTDOWN = 14;
static const uint8_t PIN_OOK      =  5;

static RFSI4463PRO rf(PIN_SS, PIN_SHUTDOWN);


void SendCombination(uint16_t combo10bit)
{
    Pin pinOOK(PIN_OOK, LOW);

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

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    rf.Init();
    const uint32_t FREQ = 310000000;
    rf.SetFrequency(FREQ);
    rf.SetModemAsyncOOK();
    rf.StartTx();

    const uint16_t COMBO = 0b0000001110000111;

    while (1)
    {
        SendCombination(COMBO);

        PAL.Delay(1000);
    }
}



void loop() { }












