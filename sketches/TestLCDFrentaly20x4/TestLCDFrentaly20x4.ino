#include "Log.h"
#include "LCDFrentaly20x4.h"

static LCDFrentaly20x4 lcd;

void setup()
{
    LogStart(9600);
    Log("Starting");
    
    lcd.Init();

    lcd.PrintAt(0, 0, "01234567890123456789");
    lcd.PrintAt(0, 1, "|   Hello          |");
    lcd.PrintAt(0, 2, "|         World!   |");
    lcd.PrintAt(0, 3, "01234567890123456789");
    
    char spinCharList[] = { '^', '>', 'v', '<' };
    uint8_t spinCharListLen = sizeof(spinCharList);
    uint8_t idx = 0;
    
    while (1)
    {
        Log("Next");
        
        idx = (idx + 1) % spinCharListLen;
        
        char c = spinCharList[idx];

        lcd.PrintAt( 0, 1, c);
        lcd.PrintAt(19, 1, c);
        
        lcd.PrintAt( 0, 2, c);
        lcd.PrintAt(19, 2, c);
        
        delay(250);
    }
}

void loop() { }



