#include "ProtocolOneWire.h"

static const uint8_t PIN = 15;

static ProtocolOneWire ow(PIN);

void setup()
{
    @fix@Serial.begin(9600);

    ow.Init();
    
    while (1)
    {
        ow.ResetSignal();

        PAL.Delay(500);


        ow.CmdReadRom();
        
        PAL.Delay(500);

        
        ow.CmdSkipRom();
        
        PAL.Delay(500);
    }
}

void loop() {}




