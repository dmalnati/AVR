// rf4463_tx.pde

// Caution:RF4463 can only work under 3.3V
// please make sure the supply of you board is under 3.3V
// 5v supply will destroy RF4463 module!!

// This code runs in tx mode and  works with rf4463_rx.pde 
// Flow:receive "T" from serial->send a packet
// data of packet is "swwxABCDEFGHIm"

#include<RF4463.h>
#include <SPI.h>
#include <SoftwareSerial.h>
RF4463 rf4463;
//unsigned char tx_buf[]={"swwxABCDEFGHIm"};
char tx_buf[64];
unsigned char val;
unsigned char flag=0;    //  flag of rx mode

void PopulateData()
{
    memset(tx_buf, (uint8_t)'A', sizeof(tx_buf));
}

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
   PopulateData();
   rf4463.enterStandbyMode();
}
void loop() 
{
    val=Serial.read();  // please make sure serial is OK befor runing this code
    if(val=='T')    // tx a packet if receive "T"
    {
       Serial.print("Sending ");
       Serial.print(sizeof(tx_buf));
       Serial.println(" bytes of data");
       uint32_t startTime = micros();
       rf4463.txPacket((unsigned char *)tx_buf,sizeof(tx_buf));
       uint32_t txTime = micros();
       //rf4463.enterStandbyMode();
       uint32_t standbyTime = micros();

       uint32_t totalDuration   = standbyTime - startTime;
       uint32_t txDuration      = txTime - startTime;
       uint32_t standbyDuration = standbyTime - txTime;

       Serial.print("tx: ");
       Serial.print(txDuration);
       Serial.println(" us");
       Serial.print("standby: ");
       Serial.print(standbyDuration);
       Serial.println(" us");
       Serial.print("total: ");
       Serial.print(totalDuration);
       Serial.println(" us");
       Serial.println();
    }
}
