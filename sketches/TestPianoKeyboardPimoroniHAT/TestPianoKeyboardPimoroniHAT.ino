#include "Evm.h"
#include "Function.h"
#include "PianoKeyboardPimoroniHAT.h"

Evm::Instance<10,10,10> evm;
PianoKeyboardPimoroniHAT hat;


void setup()
{
    Serial.begin(9600);

    Serial.println("Starting");
    
    hat.SetCallbackOnKeyDown([](uint8_t key){
        Serial.print("OnKeyDown: "); Serial.println(key);
    });
    hat.SetCallbackOnKeyUp([](uint8_t key){
        Serial.print("OnKeyUp  : "); Serial.println(key);
    });
    hat.SetCallbackOnInstrumentChangeKeyPress([](){
        Serial.println("OnInstrumentChangeKeyPress");
    });
    hat.SetCallbackOnOctaveKeyUpPress([](){
        Serial.println("OnOctaveKeyUpPress");
    });
    hat.SetCallbackOnOctaveKeyDownPress([](){
        Serial.println("OnOctaveKeyDownPress");
    });

    hat.EnableLEDs();
    hat.Init();

    evm.MainLoop();
}

void loop() {}



