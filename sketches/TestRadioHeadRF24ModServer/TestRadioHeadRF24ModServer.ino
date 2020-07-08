// rf24_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing server
// with the RH_RF24 class. RH_RF24 class does not provide for addressing or
// reliability, so you should only use RH_RF24  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf24_client
// Tested on Anarduino Mini http://www.anarduino.com/mini/ with RFM24W and RFM26W

#include "Evm.h"
#include "TimedEventHandler.h"

#include <SPI.h>
#include <RH_RF24_mod.h>

static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;

// Singleton instance of the radio driver
static const uint8_t PIN_IRQ = 12;  // mod supports real pin 12;
static const uint8_t PIN_SDN = 13;  // mod supports real pin 13;
static const uint8_t PIN_SEL = 14;  // mod supports real pin 14;
RH_RF24_mod rf24(PIN_SEL, PIN_IRQ, PIN_SDN);


void setup() 
{
  LogStart(9600);
  if (!rf24.init())
    Log("init failed");
  // The default radio config is for 30MHz Xtal, 434MHz base freq 2GFSK 5kbps 10kHz deviation
  // power setting 0x10
  // If you want a different frequency mand or modulation scheme, you must generate a new
  // radio config file as per the RH_RF24 module documentation and recompile
  // You can change a few other things programatically:
  //rf24.setFrequency(435.0); // Only within the same frequency band
  //rf24.setTxPower(0x7f);
}

void loop()
{
    ted.SetCallback([](){

    
  if (rf24.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF24_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf24.recv(buf, &len))
    {
//      RF24::printBuffer("request: ", buf, len);
      Log("got request: ", (char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println((uint8_t)rf24.lastRssi(), DEC);
      
      // Send a reply
      uint8_t data[] = "And hello back to you";
      rf24.send(data, sizeof(data));
      rf24.waitPacketSent();
      Log("Sent a reply");
    }
    else
    {
      Log("recv failed");
    }
  }

    });
  ted.RegisterForTimedEventInterval(400, 0);

  Log("MainLoop");
  Log("");
  evm.MainLoop();
    
}
