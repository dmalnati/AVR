#include "Evm.h"
#include "PAL.h"
#include "Serial.h"


static Evm::Instance<10,10,10> evm;
static Serial0 s;
static TimedEventHandlerDelegate ted;


void setup()
{
    s.Start(9600);

    ted.SetCallback([](){
        //s.Start(9600);
        //s.Write("hey there\n");

        char buf[50];
        uint8_t avail = s.Available();
        itoa(avail, buf, 10);

        s.Write((uint8_t *)buf, strlen(buf));
        s.Write('\n');

        for (uint8_t i = 0; i < avail; ++i)
        {
            uint8_t b = s.Read();
            
            s.Write(b);
        }
        s.Write('\n');
        
        //s.Stop();
    });

    ted.RegisterForTimedEventInterval(2000);

    evm.MainLoop();
}

void loop()
{

}
