#ifndef __DISPLAY_AND_MENU_H__
#define __DISPLAY_AND_MENU_H__


#include "SimpleString.h"
#include "Function.h"
#include "Evm.h"
#include "LCDFrentaly20x4.h"
#include "Keypad12Button.h"

 
/*
 * 
 * Menu items:
 * - status (live updating)
 * - (re)initialize (find boundaries)
 *   - command/action, only option is to run it
 *     - should it update while initializing?
 *     - probably a normal thing to want, the status of a command...
 * - run/start (apply configuration and operate)
 *   - command/action, only option is to run
 *     - should display info as to next step countdown, remaining, etc
 * - stop
 *   - command/action, only option is to stop
 * - configure step size / speed
 *   - input, numeric input
 * - 
 * 
 * 
 * How Display works with user input
 * - main screen, shows status
 * - press # key to get into menu system, and go further into menus, and acts as enter key
 * - press * key to cancel input, back up out of menus
 * 
 * 
 * 
 * 
 * Implementation notes
 * - Status can be updated at any time
 *   - hmm, maybe Display should have to call a function to ask for display
 *   - app code then simply tells Display it's ready to show something new
 *     - that way we don't overwrite one of the sub-menus, and don't need to
 *       store a lot of text state, it is generated on-demand
 * - Let that be the way every menu element works also
 * - Input screens
 *   - they don't get to choose how they're displayed
 *   - instead, Display shows description, old value, new value
 *   - input type can be enum or numeric
 *     - on #, fn called, retVal indicates ok or not, which is displayed to user
 * - 
 * 
 * 
 * 
 * 
 * Main display
 * Menus
 * - commands
 * - configuration
 *   - enumerated values
 *   - user input
 *
 * Main display runs continuously in the background.  Requests to update it
 * may or may not be honored if a menu is active.
 *
 * You can scroll through menu options.
 * You can select a menu option and input a value.
 *
 * A command menu option, when invoked, will kick you back to the main screen
 * and you will watch progress from there.
 *
 *
 *
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */


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




template <uint8_t COUNT_MENU_ITEMS, uint8_t COUNT_CHAR_INPUT = 8>
class DisplayAndMenu
{
public:
    DisplayAndMenu()
    : activeScreen_(ActiveScreen::MAIN_SCREEN)
    , menuItemActive_(MenuItemActive::INACTIVE)
    , menuItemListIdx_(0)
    , kpad_({
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19
    })
    {
        // Nothing to do
    }
    
    void SetFnRedrawMainScreen(function<void(LCDFrentaly20x4 &lcd)> &&fnRedrawMainScreen)
    {
        fnRedrawMainScreen_ = fnRedrawMainScreen;
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
        menuItemListIdx_ = (menuItemListIdx_ - 1) % COUNT_MENU_ITEMS;
        
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
    }

    void ApplyAccumulatedInput()
    {
        MenuItem &menuItem = menuItemList_[menuItemListIdx_];

        ((MenuItemInputNum &)menuItem).fnOnInput(inputStr_.UnsafePtr());
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
            lcd_.PrintAt(0, 0, "# to control Command: ");
            lcd_.PrintAt(4, 1, mic.description);
        }
        else
        {
            const char *prefix = "Command: ";
            lcd_.PrintAt(0, 0, prefix);
            lcd_.PrintAt(0, strlen(prefix), mic.description);
        }
        
        DrawFooter();
    }
    
    void DrawMenuItemInputNum(MenuItemInputNum &miin)
    {
        if (menuItemActive_ == MenuItemActive::INACTIVE)
        {
            lcd_.PrintAt(0, 0, "# to control Input: ");
            lcd_.PrintAt(4, 1, miin.description);
        }
        else
        {
            const char *prefix = NULL;
            
            prefix = "Input: ";
            lcd_.PrintAt(0,              0, prefix);
            lcd_.PrintAt(strlen(prefix), 0, miin.description);
            
            prefix = "Val: ";
            lcd_.PrintAt(4, 1, prefix);
            lcd_.PrintAt(4 + strlen(prefix), 1, inputStr_.UnsafePtr());
        }
        
        DrawFooter();
    }
    
    void DrawFooter()
    {
        lcd_.PrintAt(0, 3, "* back, # select");
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

    MenuItem menuItemList_[COUNT_MENU_ITEMS];
    uint8_t  menuItemListIdx_;

    SimpleString<COUNT_CHAR_INPUT> inputStr_;

    LCDFrentaly20x4 lcd_;
    Keypad12Button  kpad_;
};






#endif  // __DISPLAY_AND_MENU_H__






















