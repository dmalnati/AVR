#ifndef __DISPLAY_AND_MENU_H__
#define __DISPLAY_AND_MENU_H__


#include "Evm.h"
#include "Function.h"
#include "TimedEventHandler.h"
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
 * (really?)
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
    void (*fnRedraw)();
    char *description;
};

struct MenuItemRange
{
    void (*fnRedraw)();
    char *description;
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
       
        data_.mic = mic;
    }
   
    MenuItem(MenuItemRange &&mir)
    {
        type_ = MenuItemType::RANGE;
       
        data_.mir = mir;
    }

private: 
    enum MenuItemType
    {
        COMMAND = 0,
        RANGE
    } type_;
   
    union
    {
        MenuItemCommand mic;
        MenuItemRange   mir;
    } data_;
};




template <uint8_t COUNT_MENU_ITEMS, uint8_t COUNT_CHAR_INPUT = 8>
class DisplayAndMenu
: private TimedEventHandler
{
public:
    DisplayAndMenu()
    : activeScreen_(ActiveScreen::MAIN_SCREEN)
    , menuItemActive_(MenuItemActive::INACTIVE)
    , menuItemListIdx_(0)
    , inputBuf_{0}
    , inputBufIdx_(0)
    , kpad_({
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19
    })
    {
        // Nothing to do
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
        // Only queue the event if the main screen is currently active
        if (activeScreen_ == ActiveScreen::MAIN_SCREEN)
        {
            // Set up async callback to request redraw
            RegisterForTimedEvent(0);
        }
    }

    void RequestMenuItemScreenRedraw(MenuItemHandle menuItemHandle)
    {
        if (activeScreen_    == ActiveScreen::MENU_ITEM &&
            menuItemListIdx_ == menuItemHandle)
        {
            RegisterForTimedEvent(0);
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
                if (1 /*enum input*/)
                {
                    switch (c)
                    {
                    case '*': SwitchFromMenuItemActiveToMenuItemSelection(); break;
                    case '#': ApplyEnumValue();                              break;
                    case '4': ShowPrevEnumValue();                           break;
                    case '6': ShowNextEnumValue();                           break;
                    
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
        activeScreen_ = ActiveScreen::MENU_ITEM;
        RegisterForTimedEvent(0);
    }

    void ShowPrevMenuItem()
    {
        
    }

    void ShowNextMenuItem()
    {
        
    }

    void ShowPrevEnumValue()
    {
        
    }

    void ShowNextEnumValue()
    {
        
    }

    void ApplyEnumValue()
    {
        
    }

    void AccumulateInput(char /*c*/)
    {
        
    }

    void ApplyAccumulatedInput()
    {
        
    }

    void SwitchFromMenuItemSelectionToMenuItemActive()
    {
        
    }

    void SwitchFromMenuItemActiveToMenuItemSelection()
    {
        
    }

    void SwitchFromMenuItemSelectionToMainScreen()
    {
        
    }


    // Called when a screen has requested to be redrawn
    virtual void OnTimedEvent()
    {
        if (activeScreen_ == ActiveScreen::MAIN_SCREEN)
        {
            fnRedrawMainScreen_();
        }
        else
        {
            RedrawMenuItem();
        }
    }

    void RedrawMenuItem()
    {
        MenuItem &menuItem = menuItemList_[menuItemListIdx_];

        switch (menuItem.type_)
        {
        case MenuItem::MenuItemType::COMMAND: menuItem.data_.mic.fnRedraw(); break;

        default: break;
        }
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
    

    function<void()> fnRedrawMainScreen_;

    MenuItem menuItemList_[COUNT_MENU_ITEMS];
    uint8_t  menuItemListIdx_;

    char    inputBuf_[COUNT_CHAR_INPUT];
    uint8_t inputBufIdx_;

    LCDFrentaly20x4 lcd_;
    Keypad12Button  kpad_;
};






#endif  // __DISPLAY_AND_MENU_H__






















