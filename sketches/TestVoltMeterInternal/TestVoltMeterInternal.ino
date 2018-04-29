
// https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");
    
    uint32_t low  = 999999;
    uint32_t high = 0;

    uint32_t timeStart = millis();
    uint32_t intervalMs = 1000;

    while (1)
    {
        uint32_t vcc = readVcc();

        if (vcc > high) { high = vcc; }
        if (vcc < low)  { low  = vcc; }

        uint32_t timeNow = millis();

        if ((timeNow - timeStart) >= intervalMs)
        {
            @fix@Serial.print("VCC: ");
            @fix@Serial.print(vcc);
            @fix@Serial.print(", high: ");
            @fix@Serial.print(high);
            @fix@Serial.print(", low: ");
            @fix@Serial.print(low);
            @fix@Serial.println();

            high = 0;
            low  = 999999;

            timeStart = millis();
        }
    }
}

void loop() {}



