#include "PAL.h"
#include "Evm.h"


static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;

static Pin pinToggle(13, LOW);

static const uint32_t DELAY_MS = 3000;


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");
    PAL.Delay(20);

    uint8_t count = 1;
    ted.SetCallback([&](){
        if (count == 1)
        {
            evm.LowPowerDisable();
            PAL.DigitalWrite(pinToggle, HIGH);
        }
        else if (count == 2)
        {
            PAL.DigitalWrite(pinToggle, LOW);
        }
        if (count == 3)
        {
            evm.LowPowerEnable();
            PAL.DigitalWrite(pinToggle, HIGH);
        }
        else if (count == 4)
        {
            PAL.DigitalWrite(pinToggle, LOW);
        }
        
        ++count;

        if (count > 4)
        {
            count = 1;
        }
    });
    ted.RegisterForTimedEventInterval(DELAY_MS);

    evm.MainLoopLowPower();
}

void loop() {}


