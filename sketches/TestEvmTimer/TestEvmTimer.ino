#include "Evm.h"

Evm::Instance<10,10,10> evm;

TimedEventHandlerDelegate ted1;
TimedEventHandlerDelegate ted2;
TimedEventHandlerDelegate ted3;


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");
    
    uint32_t count1 = 0;
    uint32_t count2 = 0;

    ted1.SetCallback([&](){
        ++count1;
    });

    ted2.SetCallback([&](){
        ++count2;
    });

    ted3.SetCallback([&](){
        Serial.print("Count1: "); Serial.println(count1);
        count1 = 0;
        Serial.print("Count2: "); Serial.println(count2);
        count2 = 0;
    });

    ted1.RegisterForTimedEventInterval(0);
    ted2.RegisterForTimedEventInterval(0);
    ted3.RegisterForTimedEventInterval(1000);

    evm.MainLoop();
}

void loop() {}


