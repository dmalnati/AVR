#include "PAL.h"


static const uint32_t SPIN_MS  = 5000;
static const uint32_t SLEEP_MS = 30000;


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");
    
    // Just burn regular CPU on startup so it's easy to spot
    // the difference when going low power
    PAL.Delay(SPIN_MS);

    // Now go low power forever
    while (1)
    {
        PAL.DelayLowPower(SLEEP_MS);
    }
}

void loop() {}


