#include "PAL.h"
#include "Serial.h"


static Serial0 s;
static Pin dbg(15, LOW);

void setup()
{
    s.Start(9600);

    while (1)
    {
        s.Start(9600);
        s.Write("hey there\n");
        s.Stop();

        PAL.DigitalToggle(dbg);
        
        PAL.Delay(2000);
    }
}

void loop()
{

}
