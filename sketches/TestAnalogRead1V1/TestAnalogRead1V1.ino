#include "Log.h"
#include "PAL.h"


struct PinState
{
    uint8_t  pin;
    uint16_t oldVal;
    uint16_t newVal;
};

static PinState pinStateList[] = {
    { 25, 0, 0 },
};


void ReportAnalogRead(PinState &pinState)
{
    // move prior newVal to oldVal
    pinState.oldVal = pinState.newVal;

    // do read
    pinState.newVal = PAL.AnalogRead1V1(pinState.pin);

    // decide if value changed
    if (pinState.oldVal != pinState.newVal)
    {
        // 1.1 V = 1100 mV = 1100000 uV, spread across a 10 bit number
        uint16_t uvStepSize = 1100000UL / 1024;  // 1074 uV
        uint16_t mvStepSize = 1100UL / 1024;  // 1 mV

        uint32_t uvVal      = (uint32_t)uvStepSize * (uint32_t)pinState.newVal;
        uint32_t mvValCalcd = uvVal / 1000;
        uint32_t mvVal      = (uint32_t)mvStepSize * (uint32_t)pinState.newVal;
        uint32_t mvErr      = mvValCalcd - mvVal;
        
        Log(pinState.pin, ": ", pinState.newVal,
            ", uV = ", uvVal,
            ", mV(calc'd) = ", mvValCalcd,
            ", mV = ", mvVal,
            ", mvErr = ", mvErr);
    }
}

/*
 * 
 * The error of just using the raw ADC value with 1.1v reference isn't that bad
 * compared to calculating it.
 * 
 * Error is greatest at higher voltages, lowest closer to GND.
 * 
 * Depending how desperate to avoid multiply/divide, could be used.
 * 
 * Then again, if you're measuring in this resolution, you probably care about small
 * changes in voltage.
 * 
 * 
 * 
25: 1023, uV = 1098702, mV(calc'd) = 1098, mV = 1023, mvErr = 75
25: 1000, uV = 1074000, mV(calc'd) = 1074, mV = 1000, mvErr = 74
25: 1023, uV = 1098702, mV(calc'd) = 1098, mV = 1023, mvErr = 75
25: 1012, uV = 1086888, mV(calc'd) = 1086, mV = 1012, mvErr = 74
25: 983, uV = 1055742, mV(calc'd) = 1055, mV = 983, mvErr = 72
25: 957, uV = 1027818, mV(calc'd) = 1027, mV = 957, mvErr = 70
25: 958, uV = 1028892, mV(calc'd) = 1028, mV = 958, mvErr = 70
25: 957, uV = 1027818, mV(calc'd) = 1027, mV = 957, mvErr = 70
25: 818, uV = 878532, mV(calc'd) = 878, mV = 818, mvErr = 60
25: 829, uV = 890346, mV(calc'd) = 890, mV = 829, mvErr = 61
25: 817, uV = 877458, mV(calc'd) = 877, mV = 817, mvErr = 60
25: 808, uV = 867792, mV(calc'd) = 867, mV = 808, mvErr = 59
25: 800, uV = 859200, mV(calc'd) = 859, mV = 800, mvErr = 59
25: 787, uV = 845238, mV(calc'd) = 845, mV = 787, mvErr = 58
25: 785, uV = 843090, mV(calc'd) = 843, mV = 785, mvErr = 58
25: 773, uV = 830202, mV(calc'd) = 830, mV = 773, mvErr = 57
25: 764, uV = 820536, mV(calc'd) = 820, mV = 764, mvErr = 56
25: 751, uV = 806574, mV(calc'd) = 806, mV = 751, mvErr = 55
25: 722, uV = 775428, mV(calc'd) = 775, mV = 722, mvErr = 53
25: 702, uV = 753948, mV(calc'd) = 753, mV = 702, mvErr = 51
25: 690, uV = 741060, mV(calc'd) = 741, mV = 690, mvErr = 51
25: 672, uV = 721728, mV(calc'd) = 721, mV = 672, mvErr = 49
25: 656, uV = 704544, mV(calc'd) = 704, mV = 656, mvErr = 48
25: 609, uV = 654066, mV(calc'd) = 654, mV = 609, mvErr = 45
25: 568, uV = 610032, mV(calc'd) = 610, mV = 568, mvErr = 42
25: 527, uV = 565998, mV(calc'd) = 565, mV = 527, mvErr = 38
25: 517, uV = 555258, mV(calc'd) = 555, mV = 517, mvErr = 38
25: 502, uV = 539148, mV(calc'd) = 539, mV = 502, mvErr = 37
25: 477, uV = 512298, mV(calc'd) = 512, mV = 477, mvErr = 35
25: 454, uV = 487596, mV(calc'd) = 487, mV = 454, mvErr = 33
25: 430, uV = 461820, mV(calc'd) = 461, mV = 430, mvErr = 31
25: 426, uV = 457524, mV(calc'd) = 457, mV = 426, mvErr = 31
25: 389, uV = 417786, mV(calc'd) = 417, mV = 389, mvErr = 28
25: 369, uV = 396306, mV(calc'd) = 396, mV = 369, mvErr = 27
25: 303, uV = 325422, mV(calc'd) = 325, mV = 303, mvErr = 22
25: 281, uV = 301794, mV(calc'd) = 301, mV = 281, mvErr = 20
25: 243, uV = 260982, mV(calc'd) = 260, mV = 243, mvErr = 17
25: 208, uV = 223392, mV(calc'd) = 223, mV = 208, mvErr = 15
25: 226, uV = 242724, mV(calc'd) = 242, mV = 226, mvErr = 16
25: 220, uV = 236280, mV(calc'd) = 236, mV = 220, mvErr = 16
25: 191, uV = 205134, mV(calc'd) = 205, mV = 191, mvErr = 14
25: 187, uV = 200838, mV(calc'd) = 200, mV = 187, mvErr = 13
25: 167, uV = 179358, mV(calc'd) = 179, mV = 167, mvErr = 12
25: 129, uV = 138546, mV(calc'd) = 138, mV = 129, mvErr = 9
25: 126, uV = 135324, mV(calc'd) = 135, mV = 126, mvErr = 9
25: 93, uV = 99882, mV(calc'd) = 99, mV = 93, mvErr = 6
25: 85, uV = 91290, mV(calc'd) = 91, mV = 85, mvErr = 6
25: 71, uV = 76254, mV(calc'd) = 76, mV = 71, mvErr = 5
25: 47, uV = 50478, mV(calc'd) = 50, mV = 47, mvErr = 3
25: 32, uV = 34368, mV(calc'd) = 34, mV = 32, mvErr = 2
25: 4, uV = 4296, mV(calc'd) = 4, mV = 4, mvErr = 0
25: 1, uV = 1074, mV(calc'd) = 1, mV = 1, mvErr = 0

 */

void setup()
{
    LogStart(9600);
    Log("Starting");

    while (1)
    {
        for (auto &pinState : pinStateList)
        {
            ReportAnalogRead(pinState);
        }
    }
}

void loop() {}



