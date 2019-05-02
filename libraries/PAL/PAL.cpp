#include "PAL.h"


//////////////////////////////////////////////////////////////////////
//
// Local Storage
//
//////////////////////////////////////////////////////////////////////

#ifdef __AVR_ATmega328P__


/*
 *
 * Registers:
 * - This chip has 3 ports - B, C, D
 * - Where X is used below, use one of B, C, D for relevant operation
 *
 * DDRX - Data Direction - control whether a pin is input or output
 *        (0 = INPUT, 1 = OUTPUT)
 * 
 * When DDR indicates a pin is input:
 * - PORTX dictates whether there is an internal pullup
 * - PINX  reads from the pin
 * 
 * When DDR indicates a pin is output:
 * - PORTX writes to the pin
 * - PINX  toggles the value on the pin
 *
 */
volatile uint8_t *PlatformAbstractionLayer::port__ddrxPtr[3]  = { &DDRB,  &DDRC,  &DDRD   };
volatile uint8_t *PlatformAbstractionLayer::port__pinxPtr[3]  = { &PINB,  &PINC,  &PIND   };
volatile uint8_t *PlatformAbstractionLayer::port__portxPtr[3] = { &PORTB, &PORTC, &PORTD  };


PlatformAbstractionLayer::WdtData PlatformAbstractionLayer::wdtDataArr_[10] = {
    { WatchdogTimeout::TIMEOUT_15_MS,     15, 0 },
    { WatchdogTimeout::TIMEOUT_30_MS,     30, 0 },
    { WatchdogTimeout::TIMEOUT_60_MS,     60, 0 },
    { WatchdogTimeout::TIMEOUT_120_MS,   120, 0 },
    { WatchdogTimeout::TIMEOUT_250_MS,   250, 0 },
    { WatchdogTimeout::TIMEOUT_500_MS,   500, 0 },
    { WatchdogTimeout::TIMEOUT_1000_MS, 1000, 0 },
    { WatchdogTimeout::TIMEOUT_2000_MS, 2000, 0 },
    { WatchdogTimeout::TIMEOUT_4000_MS, 4000, 0 },
    { WatchdogTimeout::TIMEOUT_8000_MS, 8000, 0 },
};

volatile uint8_t PlatformAbstractionLayer::watchdogTimeoutCalibrationTimeoutFlag_ = 0;

function<void()>                  PlatformAbstractionLayer::cbFnWDT_;
PlatformAbstractionLayer::CbFnRaw PlatformAbstractionLayer::cbFnRawWDT_ = PlatformAbstractionLayer::OnFnRawWDTDefault;


ISR(WDT_vect)
{
    PlatformAbstractionLayer::cbFnRawWDT_();
}


#endif  // __AVR_ATmega328P__



// Globally-available instance.
PlatformAbstractionLayer PAL;



