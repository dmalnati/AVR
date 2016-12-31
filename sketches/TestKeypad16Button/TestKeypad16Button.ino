#include "Evm.h"
#include "Keypad16Button.h"


static Evm::Instance<10,10,10> evm;

static Keypad16Button kpad({
    11, 12, 13, 14, 15, 16, 17, 18
});


void setup()
{
    Serial.begin(9600);

    Serial.println("Starting");
    
    kpad.Init([](char c) {
        Serial.println(c);
    });

    evm.MainLoop();
}

void loop() {}






