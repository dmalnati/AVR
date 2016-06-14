#include <avr/interrupt.h>

#include "Ivm.h"


extern volatile uint8_t  port__pinStateLast[];


// ISR for Port B
ISR(PCINT0_vect)
{
    // Cache prior value for handoff
    uint8_t pinStateLast = port__pinStateLast[PlatformAbstractionLayer::PORT_B];
    
    // Store current state for next time
    port__pinStateLast[PlatformAbstractionLayer::PORT_B] = PINB;
    
    // Pass upward
    Ivm::OnISR(PlatformAbstractionLayer::PORT_B, PINB, pinStateLast);
}

// ISR for Port C
ISR(PCINT1_vect)
{
    // Cache prior value for handoff
    uint8_t pinStateLast = port__pinStateLast[PlatformAbstractionLayer::PORT_C];
    
    // Store current state for next time
    port__pinStateLast[PlatformAbstractionLayer::PORT_C] = PINC;
    
    // Pass upward
    Ivm::OnISR(PlatformAbstractionLayer::PORT_C, PINC, pinStateLast);
}

// ISR for Port D
ISR(PCINT2_vect)
{
    // Cache prior value for handoff
    uint8_t pinStateLast = port__pinStateLast[PlatformAbstractionLayer::PORT_D];
    
    // Store current state for next time
    port__pinStateLast[PlatformAbstractionLayer::PORT_D] = PIND;
    
    // Pass upward
    Ivm::OnISR(PlatformAbstractionLayer::PORT_D, PIND, pinStateLast);
}

// ISR for uncaught ISR
ISR(BADISR_vect)
{
    Ivm::OnBADISR();
}

