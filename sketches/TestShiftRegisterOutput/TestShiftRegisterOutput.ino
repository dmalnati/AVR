#include "ShiftRegisterOutput.h"


static const uint8_t SHIFT_REGISTERS = 2;


static const uint8_t PIN_CLOCK  = 23;
static const uint8_t PIN_LATCH  = 24;
static const uint8_t PIN_SERIAL = 25;

static ShiftRegisterOut sr(PIN_CLOCK, PIN_LATCH, PIN_SERIAL);
static ShiftRegisterOutput<SHIFT_REGISTERS> sroo(sr);


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    Write(SHIFT_REGISTERS);
}

void Write(const uint8_t srCount)
{
    ShiftRegisterOutputPin pinList[srCount * 8];

    for (uint8_t i = 0; i < (srCount * 8); ++i)
    {
        pinList[i] = sroo.GetPin(i);
    }

    while (1)
    {
        uint8_t toggleVal = 0;
        uint8_t loopCount = 4;
        while (loopCount--)
        {
            for (uint8_t i = 0; i < (srCount * 8); ++i)
            {
                #if 0
                @fix@Serial.print("Pin ");
                @fix@Serial.print(i);
                @fix@Serial.print(", val: ");
                @fix@Serial.print(i % 2 ? toggleVal : !toggleVal);
                @fix@Serial.println();
                #endif
                
                pinList[i].DigitalWrite(i % 2 ? toggleVal : !toggleVal);
            }
    
            toggleVal = !toggleVal;
    
    
            PAL.Delay(200);
        }

        for (uint8_t i = 0; i < (srCount * 8); ++i)
        {
            pinList[i].DigitalWrite(LOW);
        }

        uint8_t pinLast = 0;
        loopCount = 4;
        while (loopCount--)
        {
            for (uint8_t i = 0; i < (srCount * 8); ++i)
            {
                pinList[pinLast].DigitalWrite(LOW);
                pinList[i].DigitalWrite(HIGH);
    
                PAL.Delay(100);
    
                pinLast = i;
            }            
        }
    }
}

void loop() {}



