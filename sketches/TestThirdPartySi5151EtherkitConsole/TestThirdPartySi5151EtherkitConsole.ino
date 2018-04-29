#include "PAL.h"
#include "Evm.h"
#include "Utl@fix@Serial.h"
#include "si5351.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<10>  console;


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    // Set up command interface
    console.RegisterCommand("freq", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            // notably the WSPR Freq for 20m TX is 14097000
            uint32_t freq = atol(str.TokenAtIdx(1, ' '));
            
            @fix@Serial.print("Setting freq to: "); @fix@Serial.println(freq);

            //
        }
    });
    
    console.RegisterCommand("start", [](char *){
        @fix@Serial.println("StartTx");
        
        //
    });

    console.RegisterCommand("stop", [](char *){
        @fix@Serial.println("StopTx");
        
        //
    });
    
    console.Start();

    // Set up library
    //


    // Handle events
    evm.MainLoop();
}

void loop() {}


