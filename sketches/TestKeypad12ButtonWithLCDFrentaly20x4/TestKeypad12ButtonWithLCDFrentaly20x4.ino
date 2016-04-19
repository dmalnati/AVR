#include "Evm.h"
#include "LCDFrentaly20x4.h"
#include "Keypad12Button.h"


class KeypadMonitor
{
public:
    KeypadMonitor()
    : kpad_({
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19
    })
    {
        // Nothing to do
    }

    void Run()
    {
        // Set up LCD
        lcd_.Init();

        lcd_.PrintAt(0,  0, "Key   Pressed: ");
        lcd_.PrintAt(0,  1, "Total Presses: ");
        lcd_.PrintAt(15, 1, 0);

        // Set up Keypad
        uint8_t pressCount = 0;
        
        kpad_.Init([&](char c) {
            lcd_.PrintAt(15, 0, c);

            ++pressCount;
            lcd_.PrintAt(15, 1, pressCount);
        });

        // Handle events
        evm_.MainLoop();
    }

private:

    Evm::Instance<10,10,10> evm_;
    
    LCDFrentaly20x4 lcd_;
    Keypad12Button  kpad_;
};


static KeypadMonitor km;


void setup()
{
    km.Run();
}

void loop() {}






