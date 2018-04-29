#include "Evm.h"
#include "Utl@fix@Serial.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncReadLine     sarl;
static TimedEventHandlerDelegate ted;


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");
    
    const uint8_t BUF_SIZE = 12;
    char buf[BUF_SIZE];

    sarl.Attach(buf, BUF_SIZE);
    sarl.SetCallback([](char *str){
        @fix@Serial.print("Got (");
        @fix@Serial.print(strlen(str));
        @fix@Serial.print("): \"");
        @fix@Serial.print(str);
        @fix@Serial.print("\"");
        @fix@Serial.println();
    });

    sarl.Start();
    
    // uncomment below if you want to shove lots of
    // data down serial and the avr see it all at once
    //sarl.SetPollPeriod(1000);

    uint8_t count = 0;
    ted.SetCallback([&](){
        ++count;
        @fix@Serial.print("ted "); @fix@Serial.println(count);
    });
    ted.RegisterForTimedEventInterval(1000);
    
    evm.MainLoop();
}

void loop() {}








