#include "AppStepperSlider.h"


static AppStepperSliderConfig cfg = {
    // Stepper control parameters
    .pinEnable   = 0,
    .pinPhase1S1 = 26,
    .pinPhase1S2 = 25,
    .pinPhase2S1 = 24,
    .pinPhase2S2 = 23,
    
    // Limit switch parameters
    .pinLimitSwitchLeft  = 5,
    .pinLimitSwitchRight = 6,
    
    // LCD and Keypad parameters
    .damConfig = {
        .keypadConfig = {
            10, 11, 12, 13, 14, 15, 16, 17, 18, 19
        }
    }
};

static AppStepperSlider app(cfg);


void setup() 
{
    app.Run();
}

void loop() { }



