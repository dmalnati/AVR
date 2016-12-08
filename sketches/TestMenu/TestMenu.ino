#include "Evm.h"
#include "DisplayAndMenu.h"


struct DisplayUserConfig
{
    DisplayAndMenuConfig damConfig;
};


// class size ~350 bytes
class DisplayUser
: private TimedEventHandler
{
private:
    static const uint8_t COUNT_MENU_ITEMS = 4;

    using DisplayAndMenuClass = DisplayAndMenu<COUNT_MENU_ITEMS>;
    
public:
    static const uint8_t C_IDLE  = DisplayAndMenuClass::C_IDLE;
    static const uint8_t C_TIMED = DisplayAndMenuClass::C_TIMED + 1;
    static const uint8_t C_INTER = DisplayAndMenuClass::C_INTER;

public:
    DisplayUser(DisplayUserConfig &cfg)
    : disp_(cfg.damConfig)
    , counterTimedEvent_(0)
    , counterDrawScreen_(0)
    , counterCommand_(0)
    , inputVal_(0)
    , mainScreenInput_('-')
    {
        disp_.AddMenuItem(MenuItemCommand{
            .description = "CommandName1",
            .fnOnCommand = [this](){ OnCommand(); }
        });
    
        disp_.AddMenuItem(MenuItemInputNum{
            .description = "InputNumName1",
            .fnOnInput = [this](char *input){ OnInput(input); }
        });

        disp_.AddMenuItem(MenuItemCommand{
            .description = "CommandName2",
            .fnOnCommand = [this](){ OnCommand(); }
        });
    
        disp_.AddMenuItem(MenuItemInputNum{
            .description = "InputNumName2",
            .fnOnInput = [this](char *input){ OnInput(input); }
        });

        disp_.SetFnRedrawMainScreen([this](LCDFrentaly20x4 &lcd){ DrawScreen(lcd); });
        disp_.SetFnMainScreenInput([this](char c){ OnMainScreenInput(c); });
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

        // approx 128ms
        disp_.RequestMainScreenRedraw();
    }

    void OnMainScreenInput(char c)
    {
        mainScreenInput_ = c;

        disp_.RequestMainScreenRedraw();
    }

    void OnCommand()
    {
        ++counterCommand_;
    }

    void OnInput(char *input)
    {
        inputVal_ = atoi(input);
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

        prefix = "Cmd: ";
        lcd.PrintAt(0, 3, prefix);
        lcd.PrintAt(strlen(prefix), 3, counterCommand_);

        prefix = "Val: ";
        lcd.PrintAt(8, 3, prefix);
        lcd.PrintAt(8 + strlen(prefix), 3, inputVal_);

        lcd.PrintAt(17, 3, "C:");
        lcd.PrintAt(19, 3, mainScreenInput_);
    }

    DisplayAndMenuClass disp_;

    uint16_t counterTimedEvent_;
    uint16_t counterDrawScreen_;
    uint16_t counterCommand_;
    uint16_t inputVal_;
    char     mainScreenInput_;
};

static Evm::Instance<DisplayUser::C_IDLE,DisplayUser::C_TIMED,DisplayUser::C_INTER> evm;

static DisplayUserConfig cfg = {
    .damConfig = {
        .keypadConfig = {
            10, 11, 12, 13, 14, 15, 16, 17, 18, 19
        }
    }
};

static DisplayUser du(cfg);


void setup()
{
    du.Init();

    evm.MainLoop();
}


void loop() {}
















