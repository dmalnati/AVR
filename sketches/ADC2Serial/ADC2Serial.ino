#include <SoftwareSerial.h>

const int PIN_TX = 3;
const int PIN_RX = -1;
const int PIN_LED = 0;
const int PIN_ADC = A2;

SoftwareSerial SS(PIN_RX, PIN_TX);

void setup() {
  pinMode(PIN_TX, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_ADC, INPUT);

  SS.begin(9600);
}

// Polling version
void MonitorVoltage()
{
  uint16_t adcVal       = 0;
  uint8_t  adcValScaled = 0;
  
  while (1)
  {
    digitalWrite(PIN_LED, HIGH);
    
    adcVal = analogRead(PIN_ADC);
    adcValScaled = adcVal / 4;
    
    SS.write(adcValScaled);
    
    digitalWrite(PIN_LED, LOW);

    delay(50);
  }
}

// Improved.  Spin as fast as possible, only announce big change.
void MonitorVoltage2()
{
  uint16_t adcVal           = 0;
  uint8_t  adcValScaled     = 0;
  uint8_t  adcValScaledLast = 0;
  bool     publish          = true;
  uint8_t  threshold        = 50;

  // measure initial value
  adcVal = analogRead(PIN_ADC);
  adcValScaled = adcVal / 4;
  
  while (1)
  {
     if (publish)
     {
       digitalWrite(PIN_LED, HIGH);
       SS.write(adcValScaled);
       digitalWrite(PIN_LED, LOW);

       adcValScaledLast = adcValScaled;
     }
    
    adcVal = analogRead(PIN_ADC);
    adcValScaled = adcVal / 4;

    publish = abs(adcValScaled - adcValScaledLast) >= threshold;
  }
}

void loop() {
  MonitorVoltage2();
}
