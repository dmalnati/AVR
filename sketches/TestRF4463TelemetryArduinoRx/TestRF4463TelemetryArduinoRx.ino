// rf4463_rx.pde

// Caution:RF4463 can only work under 3.3V
// please make sure the supply of you board is under 3.3V
// 5v supply will destroy RF4463 module!!

// This code runs in rx mode and  works with rf4463_tx.pde 
// Flow:receive packet from tx->print to serial
// data of packet is "swwxABCDEFGHIm"

#include<RF4463.h>
#include <SPI.h>
#include <SoftwareSerial.h>
RF4463 rf4463;
unsigned char flag=1;    //  flag of rx mode
unsigned char rx_len;
unsigned char rx_buf[64];

void setup() {
  Serial.begin(9600);
  if(!rf4463.init())
  {
     Serial.println("Init fail!");
  }
  else
  {
    Serial.println("Init success!");
  }
   rf4463.rxInit();    // wait for packet from tx
}
void loop() 
{
    if(flag==1)
    {
        if(rf4463.waitnIRQ())    // wait for PACKET_RX interrupt
        {
            uint32_t startTime = micros();
            rf4463.clrInterrupts();
            uint32_t clrTime = micros();
            rx_len=rf4463.rxPacket(rx_buf);  // read rx data
            uint32_t rxTime = micros();
            rf4463.rxInit();    // wait for packet from tx
            uint32_t rxInitTime = micros();

            uint32_t totalDuration  = rxInitTime - startTime;
            uint32_t clrDuration    = clrTime - startTime;
            uint32_t rxDuration     = rxTime - clrTime;
            uint32_t rxInitDuration = rxInitTime - rxTime;
       
            Serial.print("clr: ");
            Serial.print(clrDuration);
            Serial.println(" us");
            Serial.print("rx: ");
            Serial.print(rxDuration);
            Serial.println(" us");
            Serial.print("rxInit: ");
            Serial.print(rxInitDuration);
            Serial.println(" us");
            Serial.print("total: ");
            Serial.print(totalDuration);
            Serial.println(" us");
            Serial.println();
            Serial.print("Read ");
            Serial.print(rx_len);
            Serial.print(" bytes of data: ");
            Serial.write(rx_buf,rx_len);    // print out by serial
            Serial.println();
            Serial.println();
        }
    }

}
