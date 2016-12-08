#include "AppStepperSlider.h"


static AppStepperSliderConfig cfg = {
    // Stepper control parameters
    .pinEnable = 0,
    .pinPhase1S1 = 0,
    .pinPhase1S2 = 0,
    .pinPhase2S1 = 0,
    .pinPhase2S2 = 0,
    
    // Limit switch parameters
    .pinLimitSwitchLeft = 0,
    .pinLimitSwitchRight = 0,
    
    // LCD and Keypad parameters
    .damConfig = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

static AppStepperSlider app(cfg);


void setup() 
{
    app.Run();
}

void loop() { }



