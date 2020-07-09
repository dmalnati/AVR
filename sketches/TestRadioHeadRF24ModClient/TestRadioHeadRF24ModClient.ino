// rf24_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF24 class. RH_RF24 class does not provide for addressing or
// reliability, so you should only use RH_RF24 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf24_server.
// Tested on Anarduino Mini http://www.anarduino.com/mini/ with RFM24W and RFM26W

#include "Log.h"
#include "LogBlob.h"
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



void SendData()
{
    uint8_t data[] = "Hello World!";
    Log("Sending data");
    LogBlob(data, sizeof(data));
    LogNL();
    rf24.send(data, sizeof(data));
}


void setup() 
{
    LogStart(9600);
    Log("Starting");

    rf24.SetOnMessageTransmittedCallback([](){
        Log("Transmit complete");
        
        rf24.setModeRx();

        // Set timeout for waiting for reply
        uint32_t ms = 1000;
        Log("Waiting for reply for ", ms, " ms");
        LogNL();
        ted.SetCallback([](){
            Log("Haven't received response, trying again");
    
            SendData();
        });
        ted.RegisterForTimedEvent(ms);
    });
    
    rf24.SetOnMessageReceivedCallback([](uint8_t *buf, uint8_t bufLen){
        Log("Recieved reply");
        LogBlob(buf, bufLen);
        LogNL();

        // Schedule next send (which also cancels giveup timer)
        uint32_t ms = 1000;
        Log("Waiting to send again for ", ms, " ms");
        LogNL();
        ted.SetCallback([](){
            SendData();
        });
        ted.RegisterForTimedEvent(ms);
    });
        
    if (!rf24.init())
    {
        Log("Init FAIL");
    }
    else
    {
        Log("Init OK");

        SendData();
        
        Log("MainLoop");
        Log("");
        evm.MainLoop();
    }
}


void loop()
{

}
