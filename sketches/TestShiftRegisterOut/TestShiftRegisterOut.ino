#include "PAL.h"
#include "ShiftRegisterOut.h"


//static const uint8_t PIN_LOAD         = 14;
static const uint8_t PIN_CLOCK  = 13;
static const uint8_t PIN_LATCH  = 12;
static const uint8_t PIN_SERIAL = 11;

//static ShiftRegisterOut sro(PIN_LOAD, PIN_CLOCK, PIN_CLOCK_ENABLE, PIN_SERIAL);
static ShiftRegisterOut sro(PIN_CLOCK, PIN_LATCH, PIN_SERIAL);


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    Write(1);
}

void Write(const uint8_t bufSize)
{
    uint8_t buf[bufSize];
    memset(buf, 0, bufSize);

    uint8_t onOff = 1;

    while (1)
    {
        memset(buf, onOff ? 0x55 : 0xAA, bufSize);
        onOff = !onOff;
        
        sro.ShiftOut(buf, bufSize);

        PAL.Delay(200);
    }
}

void loop() {}





