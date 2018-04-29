#include "Evm.h"
#include "Function.h"
#include "PianoKeyboardPimoroniHAT.h"

Evm::Instance<10,10,10> evm;
PianoKeyboardPimoroniHAT hat;


void setup()
{
    @fix@Serial.begin(9600);

    @fix@Serial.println("Starting");
    
    hat.SetCallbackOnKeyDown([](uint8_t key){
        @fix@Serial.print("OnKeyDown: "); @fix@Serial.println(key);
    });
    hat.SetCallbackOnKeyUp([](uint8_t key){
        @fix@Serial.print("OnKeyUp  : "); @fix@Serial.println(key);
    });
    hat.SetCallbackOnInstrumentChangeKeyPress([](){
        @fix@Serial.println("OnInstrumentChangeKeyPress");
    });
    hat.SetCallbackOnOctaveKeyUpPress([](){
        @fix@Serial.println("OnOctaveKeyUpPress");
    });
    hat.SetCallbackOnOctaveKeyDownPress([](){
        @fix@Serial.println("OnOctaveKeyDownPress");
    });

    hat.EnableLEDs();
    hat.Init();

    evm.MainLoop();
}

void loop() {}



