#include <MyShiftRegister.h>
#include <SoftwareSerial.h>

const uint8_t PIN_SR_LOAD         = 0;
const uint8_t PIN_SR_CLOCK        = 1;
const uint8_t PIN_SR_CLOCK_ENABLE = 2;
const uint8_t PIN_SR_SERIAL       = 3;
const uint8_t PIN_SERIAL_TX       = 4;
const uint8_t PIN_SERIAL_RX       = -1;

MyShiftRegister sr(PIN_SR_LOAD,
                   PIN_SR_CLOCK,
                   PIN_SR_CLOCK_ENABLE,
                   PIN_SR_SERIAL);

SoftwareSerial ss(PIN_SERIAL_RX, PIN_SERIAL_TX);

void setup()
{
  pinMode(PIN_SERIAL_TX, OUTPUT);
  ss.begin(9600);
}

void MonitorSIPO()
{
  uint8_t sipoData = 0;
  uint8_t sipoDataLast = 0;
  
  while (1)
  {
    sipoData = sr.ShiftIn();

    if (sipoData != sipoDataLast)
    {
      ss.write(sipoData);
    }

    sipoDataLast = sipoData;

    // not required to delay, but saves obesrving bounce as
    // any pin toggles on and off.
    // This is purely for demo so other applications may care.
    delay(100);
  }
}

void loop()
{
  MonitorSIPO();
}








