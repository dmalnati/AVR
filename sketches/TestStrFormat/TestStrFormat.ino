#include "PAL.h"
#include "Evm.h"
#include "Utl@fix@Serial.h"
#include "StrFormat.h"


static Evm::Instance<10,10,10>         evm;
static SerialAsyncConsoleEnhanced<10>  console;


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    console.RegisterCommand("test", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));

            @fix@Serial.print(F("Testing: ")); @fix@Serial.println(val);

            char buf[3];
            buf[2] = '\0';
            
            U32ToStrPadLeft(buf, val, 2, '0');

            @fix@Serial.print(F("Result: ")); @fix@Serial.println(buf);
        }
    });

    console.Start();

    // process events
    evm.MainLoop();
}

void loop() {}




