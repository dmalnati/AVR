// rf24_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF24 class. RH_RF24 class does not provide for addressing or
// reliability, so you should only use RH_RF24 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf24_server.
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
static RH_RF24_mod rf24(PIN_SEL, PIN_IRQ, PIN_SDN);


void setup() 
{
  LogStart(9600);
  Log("Starting");
  
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

        
  Log("Sending to rf24_server");
  // Send a message to rf24_server
  uint8_t data[] = "Hello World!";
  rf24.send(data, sizeof(data));
  
  rf24.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf24.waitAvailableTimeout(500))
  { 
    // Should be a reply message for us now   
    if (rf24.recv(buf, &len))
    {
      Log("got reply: ", (char*)buf);
    }
    else
    {
      Log("recv failed");
    }
  }
  else
  {
    Log("No reply, is rf24_server running?");
  }
  Log("");
  
    });
  
  ted.RegisterForTimedEventInterval(400, 0);

  Log("MainLoop");
  Log("");
  evm.MainLoop();
}
