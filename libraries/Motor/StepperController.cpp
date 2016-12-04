#include "StepperController.h"


// http://stackoverflow.com/questions/8016780/undefined-reference-to-static-constexpr-char
constexpr StepperControllerBipolar::HState
          StepperControllerBipolar::halfStepStateList_[16];