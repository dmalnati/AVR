#include "Evm.h"
#include "UtlSerial.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncReadLine     sarl;
static TimedEventHandlerDelegate ted;


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");
    
    const uint8_t BUF_SIZE = 12;
    char buf[BUF_SIZE];

    sarl.Attach(buf, BUF_SIZE);
    sarl.SetCallback([](char *str){
        Serial.print("Got (");
        Serial.print(strlen(str));
        Serial.print("): \"");
        Serial.print(str);
        Serial.print("\"");
        Serial.println();
    });

    sarl.Start();
    
    // uncomment below if you want to shove lots of
    // data down serial and the avr see it all at once
    //sarl.SetPollPeriod(1000);

    uint8_t count = 0;
    ted.SetCallback([&](){
        ++count;
        Serial.print("ted "); Serial.println(count);
    });
    ted.RegisterForTimedEventInterval(1000);
    
    evm.MainLoop();
}

void loop() {}








