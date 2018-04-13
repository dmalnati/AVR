#include "PAL.h"
#include "Evm.h"
#include "UtlSerial.h"
#include "si5351.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<10>  console;


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    // Set up command interface
    console.RegisterCommand("freq", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            // notably the WSPR Freq for 20m TX is 14097000
            uint32_t freq = atol(str.TokenAtIdx(1, ' '));
            
            Serial.print("Setting freq to: "); Serial.println(freq);

            //
        }
    });
    
    console.RegisterCommand("start", [](char *){
        Serial.println("StartTx");
        
        //
    });

    console.RegisterCommand("stop", [](char *){
        Serial.println("StopTx");
        
        //
    });
    
    console.Start();

    // Set up library
    //


    // Handle events
    evm.MainLoop();
}

void loop() {}


