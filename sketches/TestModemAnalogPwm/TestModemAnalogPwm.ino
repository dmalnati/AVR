#include "PAL.h"
#include "ModemAnalogPwm.h"


static const uint16_t SAMPLE_RATE = 15000;
static const uint16_t FREQ1 = 1200;
static const uint16_t FREQ2 = 2200;

//static ModemAnalogPwm ma;


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    ModemAnalogPwm ma;
    
    uint16_t freqList[] = {
        FREQ1,
        FREQ2,
    };
    uint8_t idx = 0;
    
    ma.SetSampleRate(SAMPLE_RATE);

    ma.Start();
    
    while (1)
    {
        ma.SetFrequency(freqList[idx]);
        idx = !idx;
        
        PAL.Delay(1500);
    }
}

void loop() {}



