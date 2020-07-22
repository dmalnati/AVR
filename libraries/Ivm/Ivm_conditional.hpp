#include <avr/interrupt.h>

#include "Ivm.h"


extern volatile uint8_t  port__pinStateLast[];

extern volatile uint32_t TIME_US_ISR;



// ISR for Port B
ISR(PCINT0_vect)
{
    // Timestamp the event
    TIME_US_ISR = PAL.Micros();

    // Cache current pin change interrupt value and disable during ISR
    uint8_t pcmsk = PCMSK0;
    PCMSK0 = 0;

    // Capture current pin state
    uint8_t pinState = PINB;
    
    // Cache prior pin state for handoff to higher logic
    uint8_t pinStateLast = port__pinStateLast[PlatformAbstractionLayer::PORT_B];

    // Store current pin state for next time
    port__pinStateLast[PlatformAbstractionLayer::PORT_B] = pinState;
    
    // Pass upward
    Ivm::OnISR(PlatformAbstractionLayer::PORT_B, pinState, pinStateLast);

    // Restore pin change interrupt settings
    PCMSK0 = pcmsk;
}

// ISR for Port C
ISR(PCINT1_vect)
{
    // Timestamp the event
    TIME_US_ISR = PAL.Micros();

    // Cache current pin change interrupt value and disable during ISR
    uint8_t pcmsk = PCMSK1;
    PCMSK1 = 0;

    // Capture current pin state
    uint8_t pinState = PINC;
    
    // Cache prior pin state for handoff to higher logic
    uint8_t pinStateLast = port__pinStateLast[PlatformAbstractionLayer::PORT_C];

    // Store current pin state for next time
    port__pinStateLast[PlatformAbstractionLayer::PORT_C] = pinState;
    
    // Pass upward
    Ivm::OnISR(PlatformAbstractionLayer::PORT_C, pinState, pinStateLast);

    // Restore pin change interrupt settings
    PCMSK1 = pcmsk;
}

// ISR for Port D
ISR(PCINT2_vect)
{
    // Timestamp the event
    TIME_US_ISR = PAL.Micros();

    // Cache current pin change interrupt value and disable during ISR
    uint8_t pcmsk = PCMSK2;
    PCMSK2 = 0;

    // Capture current pin state
    uint8_t pinState = PIND;
    
    // Cache prior pin state for handoff to higher logic
    uint8_t pinStateLast = port__pinStateLast[PlatformAbstractionLayer::PORT_D];

    // Store current pin state for next time
    port__pinStateLast[PlatformAbstractionLayer::PORT_D] = pinState;
    
    // Pass upward
    Ivm::OnISR(PlatformAbstractionLayer::PORT_D, pinState, pinStateLast);

    // Restore pin change interrupt settings
    PCMSK2 = pcmsk;
}

// ISR for uncaught ISR
ISR(BADISR_vect)
{
    Ivm::OnBADISR();
}

