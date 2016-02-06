#include <SoftwareSerial.h>
#include <VirtualWire.h>

const int PIN_RF_TX = 3;
const int PIN_LED = PB2;
const int PIN_SERIAL_TX = PB1;
const int PIN_SERIAL_RX = -1;

SoftwareSerial SS(PIN_SERIAL_RX, PIN_SERIAL_TX);

void setup()
{
  vw_set_tx_pin(PIN_RF_TX);
  vw_setup(2000);

  pinMode(PIN_LED, OUTPUT);

  pinMode(PIN_SERIAL_TX, OUTPUT);
  SS.begin(9600);
}

void MyDelay(uint8_t ms)
{
  for (int i = 0; i < 255; ++i)
  {
    ++i; --i; ++i; --i;
  }
}

void TransmitLoop()
{
  uint8_t txBuf[2] = { 0 };
  uint8_t *seqNo = &(txBuf[0]);
  uint8_t *data  = &(txBuf[1]);

  while (1)
  {
    *seqNo += 1;
    *data = (*data + 1) % 5;

    SS.write(txBuf, 2);

    digitalWrite(PIN_LED, HIGH);
    vw_send(txBuf, 2);
    vw_wait_tx();
    digitalWrite(PIN_LED, LOW);
    
    MyDelay(100);
  }
}

void loop()
{
  TransmitLoop();
}
