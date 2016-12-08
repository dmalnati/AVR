#ifndef __DISPLAY_AND_MENU_H__
#define __DISPLAY_AND_MENU_H__


#include "SimpleString.h"
#include "Function.h"
#include "Evm.h"
#include "LCDFrentaly20x4.h"
#include "Keypad12Button.h"


struct MenuItemCommand
{
    const char       *description;
    function<void()>  fnOnCommand;
};
    
struct MenuItemInputNum
{
    const char             *description;
    function<void(char *)>  fnOnInput;
};


// forward decl
template <uint8_t COUNT_MENU_ITEMS, uint8_t COUNT_CHAR_INPUT> class DisplayAndMenu;

class MenuItem
{
private:
    template <uint8_t COUNT_MENU_ITEMS, uint8_t COUNT_CHAR_INPUT> friend class DisplayAndMenu;
    
public:
    // Default constructor, doesn't do anything, shouldn't be used
    // this way.  Purely to make having an array of them easier.
    MenuItem() { }
    
    MenuItem(MenuItemCommand &&mic)
    {
        type_ = MenuItemType::COMMAND;
       
        MenuItemCommand *p = (MenuItemCommand *)data_;
        *p = mic;
    }
    
    operator MenuItemCommand & ()
    {
        MenuItemCommand *p = (MenuItemCommand *)data_;
        
        return *p;
    }
   
    MenuItem(MenuItemInputNum &&miin)
    {
        type_ = MenuItemType::INPUT_NUM;
       
        MenuItemInputNum *p = (MenuItemInputNum *)data_;
        *p = miin;
    }
    
    operator MenuItemInputNum & ()
    {
        MenuItemInputNum *p = (MenuItemInputNum *)data_;
        
        return *p;
    }

private: 
    enum MenuItemType
    {
        COMMAND = 0,
        INPUT_NUM
    } type_;
   
    // must be the size of the largest of any type to be stored here
    uint8_t data_[sizeof(MenuItemInputNum)];
};


struct DisplayAndMenuConfig
{
    Keypad12ButtonConfig keypadConfig;
};

template <uint8_t COUNT_MENU_ITEMS, uint8_t COUNT_CHAR_INPUT = 8>
class DisplayAndMenu
{
public:
    static const uint8_t C_IDLE  = Keypad12Button::C_IDLE;
    static const uint8_t C_TIMED = Keypad12Button::C_TIMED;
    static const uint8_t C_INTER = Keypad12Button::C_INTER;
    
public:
    DisplayAndMenu(DisplayAndMenuConfig &cfg)
    : activeScreen_(ActiveScreen::MAIN_SCREEN)
    , menuItemActive_(MenuItemActive::INACTIVE)
    , menuItemListIdx_(0)
    , kpad_(cfg.keypadConfig)
    {
        // Nothing to do
    }
    
    void SetFnRedrawMainScreen(function<void(LCDFrentaly20x4 &lcd)> &&fnRedrawMainScreen)
    {
        fnRedrawMainScreen_ = fnRedrawMainScreen;
    }
    
    void SetFnMainScreenInput(function<void(char c)> &&fnMainScreenInput)
    {
        fnMainScreenInput_ = fnMainScreenInput;
    }

    using MenuItemHandle = uint8_t;

    MenuItemHandle AddMenuItem(MenuItem &&menuItem)
    {
        MenuItemHandle retVal = 0;
        
        if (menuItemListIdx_ != COUNT_MENU_ITEMS)
        {
            retVal = menuItemListIdx_;
            
            menuItemList_[menuItemListIdx_] = menuItem;
            ++menuItemListIdx_;
        }

        return retVal;
    }

    void Init()
    {
        // Start up the LCD
        lcd_.Init();
        ClearScreen();

        // Init the keypad and register for key press callbacks
        kpad_.Init([this](char c){ OnKeyPress(c); });
        
        // Reset the index which was previously used to add elements,
        // now to be used to indicate the active screen
        menuItemListIdx_ = 0;
    }

    void RequestMainScreenRedraw()
    {
        // Only honor the request if the main screen is currently active
        if (activeScreen_ == ActiveScreen::MAIN_SCREEN)
        {
            Draw();
        }
    }

private:

    void OnKeyPress(char c)
    {
        if (activeScreen_ == ActiveScreen::MAIN_SCREEN)
        {
            if (c == '#')
            {
                SwitchFromMainScreenToMenuItemSelection();
            }
            else
            {
                fnMainScreenInput_(c);
            }
        }
        else
        {
            if (menuItemActive_ == MenuItemActive::INACTIVE)
            {
                switch (c)
                {
                case '*': SwitchFromMenuItemSelectionToMainScreen();     break;
                case '#': SwitchFromMenuItemSelectionToMenuItemActive(); break;
                case '2': ShowPrevMenuItem();                            break;
                case '8': ShowNextMenuItem();                            break;
                
                default: break;
                }
            }
            else
            {
                MenuItem &menuItem = menuItemList_[menuItemListIdx_];
                
                if (menuItem.type_ == MenuItem::MenuItemType::COMMAND)
                {
                    switch (c)
                    {
                    case '*': SwitchFromMenuItemActiveToMenuItemSelection(); break;
                    case '#': ApplyCommand();                                break;
                        
                    default: break;
                    }
                }
                else
                {
                    switch (c)
                    {
                    case '*': SwitchFromMenuItemActiveToMenuItemSelection(); break;
                    case '#': ApplyAccumulatedInput();                       break;
                        
                    default: AccumulateInput(c); break;
                    }
                }
            }
        }
    }

    void SwitchFromMainScreenToMenuItemSelection()
    {
        if (COUNT_MENU_ITEMS)
        {
            activeScreen_ = ActiveScreen::MENU_ITEM;
            
            Draw();
        }
    }
    
    void SwitchFromMenuItemSelectionToMainScreen()
    {
        activeScreen_ = ActiveScreen::MAIN_SCREEN;
        
        ClearScreen();
        Draw();
    }
    
    void SwitchFromMenuItemSelectionToMenuItemActive()
    {
        ClearAccumulator();
        
        menuItemActive_ = MenuItemActive::ACTIVE;
        
        Draw();
    }

    void SwitchFromMenuItemActiveToMenuItemSelection()
    {
        menuItemActive_ = MenuItemActive::INACTIVE;
        
        Draw();
    }

    void ShowPrevMenuItem()
    {
        menuItemListIdx_ = (menuItemListIdx_ == 0) ?
                           COUNT_MENU_ITEMS - 1    :
                           menuItemListIdx_ - 1;
        
        Draw();
    }

    void ShowNextMenuItem()
    {
        menuItemListIdx_ = (menuItemListIdx_ + 1) % COUNT_MENU_ITEMS;
        
        Draw();
    }

    void AccumulateInput(char c)
    {
        inputStr_.Append(c);
        
        Draw();
    }
    
    void ApplyCommand()
    {
        MenuItem &menuItem = menuItemList_[menuItemListIdx_];

        ((MenuItemCommand &)menuItem).fnOnCommand();
        
        SwitchFromMenuItemActiveToMenuItemSelection();
    }

    void ApplyAccumulatedInput()
    {
        MenuItem &menuItem = menuItemList_[menuItemListIdx_];

        ((MenuItemInputNum &)menuItem).fnOnInput(inputStr_.UnsafePtr());
        
        SwitchFromMenuItemActiveToMenuItemSelection();
    }

    void ClearAccumulator()
    {
        inputStr_.Clear();
    }

    void Draw()
    {
        if (activeScreen_ == ActiveScreen::MAIN_SCREEN)
        {
            fnRedrawMainScreen_(lcd_);
        }
        else
        {
            ClearScreen();
            RedrawMenuItem();
        }
    }

    void RedrawMenuItem()
    {
        MenuItem &menuItem = menuItemList_[menuItemListIdx_];
        
        if (menuItem.type_ == MenuItem::MenuItemType::COMMAND)
        {
            DrawMenuItemCommand(((MenuItemCommand &)menuItem));
        }
        else
        {
            DrawMenuItemInputNum(((MenuItemInputNum &)menuItem));
        }
    }
    
    void DrawMenuItemCommand(MenuItemCommand &mic)
    {
        if (menuItemActive_ == MenuItemActive::INACTIVE)
        {
            lcd_.PrintAt(0, 0, "# to select Cmd: ");
            lcd_.PrintAt(4, 1, mic.description);
        }
        else
        {
            lcd_.PrintAt(0, 0, "# to run Cmd: ");
            lcd_.PrintAt(4, 1, mic.description);
        }
        
        DrawFooter();
    }
    
    void DrawMenuItemInputNum(MenuItemInputNum &miin)
    {
        if (menuItemActive_ == MenuItemActive::INACTIVE)
        {
            lcd_.PrintAt(0, 0, "# to select Input: ");
            lcd_.PrintAt(4, 1, miin.description);
        }
        else
        {
            lcd_.PrintAt(0, 0, "# after Input: ");
            lcd_.PrintAt(4, 1, miin.description);

            const char *prefix = NULL;
            
            prefix = "Val: ";
            lcd_.PrintAt(4,                  2, prefix);
            lcd_.PrintAt(4 + strlen(prefix), 2, inputStr_.UnsafePtr());
        }
        
        DrawFooter();
    }
    
    void DrawFooter()
    {
        lcd_.PrintAt(0, 3, "* back, # select");
    }
    
    void ClearScreen()
    {
        lcd_.Clear();
    }
    

    enum struct ActiveScreen : uint8_t
    {
        MAIN_SCREEN,
        MENU_ITEM
    };

    ActiveScreen activeScreen_;

    
    enum struct MenuItemActive : uint8_t
    {
        INACTIVE = 0,
        ACTIVE
    };

    MenuItemActive menuItemActive_;
    

    function<void(LCDFrentaly20x4 &lcd)> fnRedrawMainScreen_;
    function<void(char c)>               fnMainScreenInput_;

    MenuItem menuItemList_[COUNT_MENU_ITEMS];
    uint8_t  menuItemListIdx_;

    SimpleString<COUNT_CHAR_INPUT> inputStr_;

    LCDFrentaly20x4 lcd_;
    Keypad12Button  kpad_;
};


#endif  // __DISPLAY_AND_MENU_H__






