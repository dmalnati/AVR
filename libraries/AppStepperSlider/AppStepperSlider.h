#ifndef __APP_STEPPER_SLIDER_H__
#define __APP_STEPPER_SLIDER_H__


#include "PinInput.h"
#include "Evm.h"
#include "DisplayAndMenu.h"
#include "StepperController.h"


struct AppStepperSliderConfig
{
    // Stepper control parameters
    uint8_t pinEnable;
    uint8_t pinPhase1S1;
    uint8_t pinPhase1S2;
    uint8_t pinPhase2S1;
    uint8_t pinPhase2S2;
    
    // Limit switch parameters
    uint8_t pinLimitSwitchLeft;
    uint8_t pinLimitSwitchRight;
    
    // LCD and Keypad parameters
    DisplayAndMenuConfig damConfig;
};

class AppStepperSlider
{
public:
    static const uint8_t COUNT_MENU_ITEMS = 2;

    using DisplayAndMenuClass         = DisplayAndMenu<COUNT_MENU_ITEMS>;
    using StepperControllerAsyncClass = StepperControllerAsync<StepperControllerBipolar>;

    static const uint8_t C_IDLE  = DisplayAndMenuClass::C_IDLE +
                                   StepperControllerAsyncClass::C_IDLE;
    static const uint8_t C_TIMED = DisplayAndMenuClass::C_TIMED +
                                   StepperControllerAsyncClass::C_TIMED;
    static const uint8_t C_INTER = DisplayAndMenuClass::C_INTER +
                                   StepperControllerAsyncClass::C_INTER +
                                   2;   // PinInput

    
public:
    AppStepperSlider(AppStepperSliderConfig &cfg)
    : stepperControllerBipolar_(
        cfg.pinEnable,
        cfg.pinPhase1S1,
        cfg.pinPhase1S2,
        cfg.pinPhase2S1,
        cfg.pinPhase2S2
    )
    , stepperControllerAsync_(stepperControllerBipolar_)
    , countRedraws_(0)
    , limitSwitchLeftInput_(cfg.pinLimitSwitchLeft, LOW, LEVEL_RISING_AND_FALLING)
    , atLimitLeft_(limitSwitchLeftInput_.GetLogicLevel())
    , limitSwitchRightInput_(cfg.pinLimitSwitchRight, LOW, LEVEL_RISING_AND_FALLING)
    , atLimitRight_(limitSwitchRightInput_.GetLogicLevel())
    , countLimitSwitchToggle_(0)
    , disp_(cfg.damConfig)
    {
        // Set up limit switches
        limitSwitchLeftInput_.SetCallback([this](uint8_t logicLevel)  { OnLimitSwitchLeft(logicLevel); });
        limitSwitchRightInput_.SetCallback([this](uint8_t logicLevel) { OnLimitSwitchRight(logicLevel); });

        // Set up menu
        disp_.AddMenuItem(MenuItemCommand{
            .description = "Calibrate",
            .fnOnCommand = [this](){ OnCommandCalibrate(); }
        });
    
        disp_.AddMenuItem(MenuItemInputNum{
            .description = "InputNumName1",
            .fnOnInput = [this](char *input){ OnInput(input); }
        });

        disp_.SetFnRedrawMainScreen([this](LCDFrentaly20x4 &lcd){ OnDrawMainScreen(lcd); });
        disp_.SetFnMainScreenInput([this](char c){ OnMainScreenInput(c); });
    }
    
    void Run()
    {
        disp_.Init();
        disp_.RequestMainScreenRedraw();
        
        evm_.MainLoop();
    }

private:

    void OnLimitSwitchLeft(uint8_t logicLevel)
    {
        atLimitLeft_ = logicLevel;
        
        ++countLimitSwitchToggle_;
        
        disp_.RequestMainScreenRedraw();
    }
    
    void OnLimitSwitchRight(uint8_t logicLevel)
    {
        atLimitRight_ = logicLevel;
        
        ++countLimitSwitchToggle_;
        
        disp_.RequestMainScreenRedraw();
    }

    void OnDrawMainScreen(LCDFrentaly20x4 &lcd)
    {
        ++countRedraws_;
        
        lcd.PrintAt(0, 0, "Redraws: ");
        lcd.Print(countRedraws_);
        lcd.PrintAt(0, 1, "AtLimitLeft : ");
        lcd.Print(atLimitLeft_);
        lcd.PrintAt(0, 2, "AtLimitRight: ");
        lcd.Print(atLimitRight_);
        lcd.PrintAt(0, 3, "Toggle Count: ");
        lcd.Print(countLimitSwitchToggle_);
    }
    
    void OnMainScreenInput(char /*c*/)
    {
        // TODO
    }

    void OnCommandCalibrate()
    {
        // TODO
    }
    
    void OnInput(char */*input*/)
    {
        // TODO
    }

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
    StepperControllerBipolar    stepperControllerBipolar_;
    StepperControllerAsyncClass stepperControllerAsync_;
    
    uint8_t countRedraws_;
    
    PinInput limitSwitchLeftInput_;
    uint8_t  atLimitLeft_;
    
    PinInput limitSwitchRightInput_;
    uint8_t atLimitRight_;
    
    uint8_t countLimitSwitchToggle_;
    
    DisplayAndMenuClass disp_;
};



#endif  // __APP_STEPPER_SLIDER_H__