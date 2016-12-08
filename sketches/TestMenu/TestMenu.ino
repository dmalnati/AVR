#include "Evm.h"
#include "DisplayAndMenu.h"


class DisplayUser
: private TimedEventHandler
{
public:
    DisplayUser()
    : counterTimedEvent_(0)
    , counterDrawScreen_(0)
    {
        disp_.AddMenuItem(MenuItemCommand{
            .description = "CommandName",
            .fnOnCommand = [](){}
        });
    
        disp_.AddMenuItem(MenuItemInputNum{
            .description = "InputNumName",
            .fnOnInput = [](char *){}
        });

        disp_.SetFnRedrawMainScreen([this](LCDFrentaly20x4 &lcd){ DrawScreen(lcd); });
    }

    void Init()
    {
        disp_.Init();

        RegisterForTimedEventInterval(1000);
    }
    
private:

    virtual void OnTimedEvent()
    {
        ++counterTimedEvent_;
        
        disp_.RequestMainScreenRedraw();
    }

    void DrawScreen(LCDFrentaly20x4 &lcd)
    {
        ++counterDrawScreen_;
        
        lcd.PrintAt(0, 0, "I'm the main screen");
        
        const char *prefix = NULL;

        prefix = "Timed Events: ";
        
        lcd.PrintAt(0, 1, prefix);
        lcd.PrintAt(strlen(prefix), 1, counterTimedEvent_);

        prefix = "Draw Events : ";
        lcd.PrintAt(0, 2, prefix);
        lcd.PrintAt(strlen(prefix), 2, counterDrawScreen_);
    }

    DisplayAndMenu<2> disp_;

    uint16_t counterTimedEvent_;
    uint16_t counterDrawScreen_;
};

Evm::Instance<10,10,10> evm;
DisplayUser du;


void setup()
{
    du.Init();

    evm.MainLoop();
}


void loop() {}
















