#ifndef __APP_STEPPER_SLIDER_H__
#define __APP_STEPPER_SLIDER_H__


#include "PAL.h"
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
private:
    enum struct Direction : uint8_t
    {
        LEFT = 0,
        RIGHT
    };
    
    // Set time to slide completely in one direction to be 1 hour
    static const uint32_t DEFAULT_SLIDE_DURATION_MS = 3600000;
    
public:
    static const uint8_t COUNT_MENU_ITEMS = 4;

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
    , halfStepCount_(0)
    , slideDurationMs_(DEFAULT_SLIDE_DURATION_MS)
    , stepTimeMs_(0)
    , limitSwitchLeftInput_(cfg.pinLimitSwitchLeft, LOW, LEVEL_RISING_AND_FALLING)
    , limitSwitchRightInput_(cfg.pinLimitSwitchRight, LOW, LEVEL_RISING_AND_FALLING)
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
            .description = "SlideMS",
            .fnOnInput = [this](char *input){ OnInputSlideMS(input); }
        });
        
        disp_.AddMenuItem(MenuItemCommand{
            .description = "Start",
            .fnOnCommand = [this](){ OnCommandStart(); }
        });
        
        disp_.AddMenuItem(MenuItemCommand{
            .description = "Stop",
            .fnOnCommand = [this](){ OnCommandStop(); }
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

    void OnDrawMainScreen(LCDFrentaly20x4 &lcd)
    {
        if (!IsCalibrated())
        {
            lcd.PrintAt(0, 0, "Calibration Required");
        }
        else
        {
            lcd.PrintAt(0, 0, "Steps  : ");
            lcd.Print(halfStepCount_ / 2);
        }
        
        lcd.PrintAt(0, 1, "SlideMS: ");
        lcd.Print(slideDurationMs_);
    }
    
    void OnMainScreenInput(char /*c*/)
    {
        // TODO
    }

    void OnCommandCalibrate()
    {
        // Calculate the number of half steps between the two limit switches.
        
        // Move to left limit
        uint8_t forceRefresh = 1;
        uint8_t atLimitLeft = limitSwitchLeftInput_.GetLogicLevel(forceRefresh);
        if (!atLimitLeft)
        {
            while (!limitSwitchLeftInput_.GetLogicLevel(forceRefresh))
            {
                stepperControllerBipolar_.HalfStepCCW();
                PAL.Delay(1);
            }
        }
        
        // Count steps to right limit
        halfStepCount_ = 0;
        while (!limitSwitchRightInput_.GetLogicLevel(forceRefresh))
        {
            ++halfStepCount_;
            
            stepperControllerBipolar_.HalfStepCW();
            PAL.Delay(1);
        }
        
        // Move back to the left limit as a default initial position
        while (!limitSwitchLeftInput_.GetLogicLevel(forceRefresh))
        {
            stepperControllerBipolar_.HalfStepCCW();
            PAL.Delay(1);
        }
    }
    
    uint8_t IsCalibrated()
    {
        return halfStepCount_;
    }
    
    void OnInputSlideMS(char *input)
    {
        slideDurationMs_ = atol(input);
    }
    
    void OnCommandStart()
    {
        OnCommandStop();
        
        // Operation
        // - Bounce off left and right limits indefinitely
        // - Configure speed via time to each side, in seconds
        
        if (IsCalibrated())
        {
            limitSwitchLeftInput_.Enable();
            limitSwitchRightInput_.Enable();
            
            // Calculate time between steps in order to go a full slide length
            // in the configured period.
            stepTimeMs_ = slideDurationMs_ / halfStepCount_;
            
            if (stepTimeMs_ == 0)
            {
                stepTimeMs_ = 1;
            }
            
            GoLeft();
        }
    }
    
    void GoRight()
    {
        stepperControllerAsync_.HalfStepForeverCCW(stepTimeMs_);
    }
    
    void GoLeft()
    {
        stepperControllerAsync_.HalfStepForeverCW(stepTimeMs_);
    }
    
    void OnLimitSwitchLeft(uint8_t logicLevel)
    {
        // When the switch is hit, turn around
        if (logicLevel)
        {
            disp_.RequestMainScreenRedraw();
            
            GoRight();
        }
    }
    
    void OnLimitSwitchRight(uint8_t logicLevel)
    {
        // When the switch is hit, turn around
        if (logicLevel)
        {
            disp_.RequestMainScreenRedraw();
            
            GoLeft();
        }
    }
    
    void OnCommandStop()
    {
        stepperControllerAsync_.Stop();
        
        limitSwitchLeftInput_.Disable();
        limitSwitchRightInput_.Disable();
    }
    
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
    StepperControllerBipolar    stepperControllerBipolar_;
    StepperControllerAsyncClass stepperControllerAsync_;
    
    uint32_t halfStepCount_;
    uint32_t slideDurationMs_;
    
    uint32_t  stepTimeMs_;
    
    PinInput limitSwitchLeftInput_;
    PinInput limitSwitchRightInput_;
    
    DisplayAndMenuClass disp_;
};



#endif  // __APP_STEPPER_SLIDER_H__