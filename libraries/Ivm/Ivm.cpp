#include <stdlib.h>

#include <avr/interrupt.h>

#include "Ivm.h"


/*
 *
 * Main Principle of Architecture-specific implementation of Interrupt Handling
 * - Establish an API that any given architecture can implement.
 * - Make use of that API within the externally-visible code.
 * - Detect (via #ifdef) a particular architecture, and implement.
 *
 */

 

//////////////////////////////////////////////////////////////////////
//
// Public functions
//
//////////////////////////////////////////////////////////////////////

uint8_t
Ivm::RegisterPCIntEventHandler(PCIntEventHandler *pcieh)
{
    uint8_t retVal;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        retVal = 0;
        
        // Don't allow double registration
        if (!InterruptIsActiveForPhysicalPin(pcieh->GetPin()))
        {
            retVal = AttachInterruptForPhysicalPin(pcieh->GetPin(), pcieh);
        }
    }

    return retVal;
}

uint8_t
Ivm::DeRegisterPCIntEventHandler(PCIntEventHandler *pcieh)
{
    uint8_t retVal;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        retVal = 1;
        
        // Be sure there is a handler already active
        if (InterruptIsActiveForPhysicalPin(pcieh->GetPin()))
        {
            retVal = DetachInterruptForPhysicalPin(pcieh->GetPin());
        }
    }
    
    return retVal;
}
 

 
//////////////////////////////////////////////////////////////////////
//
// Actual Interrupt vector handling.
//
// These functions are only called from an ISR.
//
// As a result, no protection from reentrant effects is required.
//
//////////////////////////////////////////////////////////////////////

void
Ivm::OnPortPinStateChange(uint8_t port,
                          uint8_t portPin,
                          uint8_t changeDir)
{
    PCIntEventHandler *pcieh =
        GetPCIntEventHandlerByPortAndPortPin(port, portPin);
    
    if (pcieh)
    {
        PCIntEventHandler::MODE changeType = (
            changeDir                             ?
            PCIntEventHandler::MODE::MODE_RISING  :
            PCIntEventHandler::MODE::MODE_FALLING
        );
    
        if (pcieh->GetMode() == changeType ||
            pcieh->GetMode() ==
                PCIntEventHandler::MODE::MODE_RISING_AND_FALLING)
        {
            pcieh->OnPCIntEvent(changeDir);
        }
    }
}


/*
 * Static function.
 *
 * Single function called from all three ISRs.
 *
 * Dedicated to understanding which pins changed, and how.
 *
 * Bitmap logic:
 *
 * old       : 0b0011
 * new       : 0b0101
 * change    : 0b0110  (old ^ new   )
 * changeDir : 0b0100  (new & change) -- only applies to bits which changed
 *
 */
void
Ivm::OnISR(uint8_t port,
           uint8_t bitmapPortPinState,
           uint8_t bitmapPortPinStateLast)
{
    // Calculate what changed, and how
    uint8_t bitmapChange    = (uint8_t)bitmapPortPinStateLast ^ bitmapPortPinState;
    uint8_t bitmapChangeDir = (uint8_t)bitmapPortPinState     & bitmapChange;

    // Apply bitmaps to figure out which pins actually changed and how
    for (uint8_t portPin = 0; portPin < 8; ++portPin)
    {
        uint8_t pinChanged = (uint8_t)(bitmapChange & (uint8_t)_BV(portPin));
        
        if (pinChanged)
        {
            // Normalize -- Change up == 1, Change down == 0
            uint8_t changeDir =
                (uint8_t)(bitmapChangeDir & (uint8_t)_BV(portPin)) ? 1 : 0;
            
            Ivm::GetInstance().OnPortPinStateChange(port, portPin, changeDir);
        }
    }
}

 
 
 
 
 
 
//////////////////////////////////////////////////////////////////////
//
// API Implementation for specific architectures.
//
//////////////////////////////////////////////////////////////////////


#ifdef __AVR_ATmega328P__


/*
 * ATmega328P Pin Change Interrupt Support
 *
 *
 * Can see literal definitions included with Arduino avr-gcc here:
 * /Program Files (x86)/Arduino/hardware/tools/avr/avr/include/avr/iom328p.h
 *
 *
 * From datasheet:
 *
 * Port/Pin | Pin Change Int | Physical Pin
 * ---------|----------------|-------------
 *   PB0    |    PCINT0      |     14      
 *   PB1    |    PCINT1      |     15      
 *   PB2    |    PCINT2      |     16      
 *   PB3    |    PCINT3      |     17      
 *   PB4    |    PCINT4      |     18      
 *   PB5    |    PCINT5      |     19      
 *   PB6    |    PCINT6      |      9      
 *   PB7    |    PCINT7      |     10      
 * ----------------------------------------
 *   PC0    |    PCINT8      |     23      
 *   PC1    |    PCINT9      |     24      
 *   PC2    |    PCINT10     |     25      
 *   PC3    |    PCINT11     |     26      
 *   PC4    |    PCINT12     |     27      
 *   PC5    |    PCINT13     |     28      
 *   PC6    |    PCINT14     |      1      
 *    x     |       x        |     x       
 * ----------------------------------------
 *   PD0    |    PCINT16     |      2      
 *   PD1    |    PCINT17     |      3      
 *   PD2    |    PCINT18     |      4      
 *   PD3    |    PCINT19     |      5      
 *   PD4    |    PCINT20     |      6      
 *   PD5    |    PCINT21     |     11      
 *   PD6    |    PCINT22     |     12      
 *   PD7    |    PCINT23     |     13      
 * ----------------------------------------
 *
 *
 * Other Pins not mentioned above
 *
 * What   | Physical Pin
 * ---------------------
 * VCC    |      7      
 * GND    |      8      
 * AVCC   |     20      
 * AREF   |     21      
 * GND    |     22      
 *
 *
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
 * - PINX  ...
 *
 */
 
 
#define EVM_ISR_PORT_B 0
#define EVM_ISR_PORT_C 1
#define EVM_ISR_PORT_D 2


//////////////////////////////////////////////////////////////////////
//
// Local Storage
//
//////////////////////////////////////////////////////////////////////
 
 /*
 * Create 3 x 8 table to store callbacks for a given Pin Change interrupt:
 * - port    (B = 0, C = 1, D = 2)
 * - portPin (0, 1, ..., 7)
 *
 * Note: 'portPin' here refers to 3 for PB3, not the physical pin
 *
 */
static PCIntEventHandler *port_portPin__pcieh[3][8] = { NULL };

// Lookup tables for access to port-specific data in an abstracted way
static          uint8_t  port__refCount[3] = { 0,       0,       0       };
static volatile uint8_t *port__pcmskPtr[3] = { &PCMSK0, &PCMSK1, &PCMSK2 };
static volatile uint8_t *port__ddrPtr[3]   = { &DDRB,   &DDRC,   &DDRD   };

// Lookup tables for access to port-specific pin values
// in an abstracted way
static volatile uint8_t *port__pinStatePtr[3]  = { &PINB, &PINC, &PIND };
static volatile uint8_t  port__pinStateLast[3] = { 0,     0,     0     };


//////////////////////////////////////////////////////////////////////
//
// API implementation for this architecture
//
//////////////////////////////////////////////////////////////////////

uint8_t
Ivm::AttachInterruptForPhysicalPin(uint8_t            physicalPin,
                                   PCIntEventHandler *pcieh)
{
    uint8_t retVal = 0;
    
    uint8_t port;
    uint8_t portPin;
    
    // Find the port and portPin to register handler on
    if (GetPortAndPortPinFromPhysicalPin(physicalPin, &port, &portPin))
    {
        retVal = 1;
        
        // Store this handler in the lookup table
        port_portPin__pcieh[port][portPin] = pcieh;
        
        // Default to explicitly setting this pin to be an output
        *port__ddrPtr[port] |= (uint8_t)_BV(portPin);
        
        // Enable interrupts for this specific pin on this port
        *port__pcmskPtr[port] |= (uint8_t)_BV(portPin);
        
        // Set the cached pin value to the present value.
        //
        // This prevents a scenario where the prior cached version is
        // (for example) a 0, but the current pin state is actually 1.
        // The next time any pin on this port changes, the ISR logic here will
        // detect a 'change' in that pin state despite it not having happened.
        //
        // Bitwise operation steps:
        // - eliminate presence of the bit representing the portPin
        //   - aka cause it to be zero without affecting any other bit values
        // - OR that value with a bitfield where only the portPin's bit value
        //   has been filtered through
        //
        port__pinStateLast[port] = (uint8_t)
            ((  port__pinStateLast[port] & (uint8_t)~_BV(portPin) )  | // zero out bit
             ( *port__pinStatePtr[port]  & (uint8_t) _BV(portPin) ));  // combine live bit
        
        // If there were previously no interrupts registered for this port
        // then enable interrupts for this port
        if (!port__refCount[port])
        {
            PCICR |= (uint8_t)_BV(port);
        }
        
        // Increment reference count for users of this port
        ++port__refCount[port];
    }
    
    return retVal;
}

uint8_t
Ivm::DetachInterruptForPhysicalPin(uint8_t physicalPin)
{
    uint8_t retVal = 0;
    
    uint8_t port;
    uint8_t portPin;
    
    // Find the port and portPin to register handler on
    if (GetPortAndPortPinFromPhysicalPin(physicalPin, &port, &portPin))
    {
        retVal = 1;
        
        // Remove the handler from the lookup table
        port_portPin__pcieh[port][portPin] = NULL;
        
        // Change nothing about the output state of the pin
        
        // Disable interrupts for this specific pon on this port
        *port__pcmskPtr[port] &= (uint8_t)~_BV(portPin);
        
        // No need to update any last values, they won't lead to app callbacks
        
        // Decrement reference count for users of this port
        --port__refCount[port];
        
        // If there are now no interrupts registered for this port
        // then disable interrupts for this port
        if (!port__refCount[port])
        {
            PCICR &= (uint8_t)~_BV(port);
        }
    }
    
    return retVal;
}

PCIntEventHandler *
Ivm::GetPCIntEventHandlerByPortAndPortPin(uint8_t port, uint8_t portPin)
{
    return port_portPin__pcieh[port][portPin];
}

uint8_t
Ivm::InterruptIsActiveForPhysicalPin(uint8_t physicalPin)
{
    uint8_t retVal = 0;
    
    uint8_t port;
    uint8_t portPin;
    
    if (GetPortAndPortPinFromPhysicalPin(physicalPin, &port, &portPin))
    {
        retVal = (GetPCIntEventHandlerByPortAndPortPin(port, portPin) != NULL);
    }
    
    return retVal;
}



// Return 1 for successfully found, 0 for failure
uint8_t
Ivm::GetPortAndPortPinFromPhysicalPin(uint8_t  physicalPin,
                                      uint8_t *port,
                                      uint8_t *portPin)
{
    uint8_t retVal = 1;
    
    switch (physicalPin)
    {
        case  0: retVal = 0;
        case  1: *port = EVM_ISR_PORT_C; *portPin = PINC6; break;
        case  2: *port = EVM_ISR_PORT_D; *portPin = PIND0; break;
        case  3: *port = EVM_ISR_PORT_D; *portPin = PIND1; break;
        case  4: *port = EVM_ISR_PORT_D; *portPin = PIND2; break;
        case  5: *port = EVM_ISR_PORT_D; *portPin = PIND3; break;
        case  6: *port = EVM_ISR_PORT_D; *portPin = PIND4; break;
        case  7: retVal = 0;                               break;
        case  8: retVal = 0;                               break;
        case  9: *port = EVM_ISR_PORT_B; *portPin = PINB6; break;
        case 10: *port = EVM_ISR_PORT_B; *portPin = PINB7; break;
        case 11: *port = EVM_ISR_PORT_D; *portPin = PIND5; break;
        case 12: *port = EVM_ISR_PORT_D; *portPin = PIND6; break;
        case 13: *port = EVM_ISR_PORT_D; *portPin = PIND7; break;
        case 14: *port = EVM_ISR_PORT_B; *portPin = PINB0; break;
        case 15: *port = EVM_ISR_PORT_B; *portPin = PINB1; break;
        case 16: *port = EVM_ISR_PORT_B; *portPin = PINB2; break;
        case 17: *port = EVM_ISR_PORT_B; *portPin = PINB3; break;
        case 18: *port = EVM_ISR_PORT_B; *portPin = PINB4; break;
        case 19: *port = EVM_ISR_PORT_B; *portPin = PINB5; break;
        case 20: retVal = 0;                               break;
        case 21: retVal = 0;                               break;
        case 22: retVal = 0;                               break;
        case 23: *port = EVM_ISR_PORT_C; *portPin = PINC0; break;
        case 24: *port = EVM_ISR_PORT_C; *portPin = PINC1; break;
        case 25: *port = EVM_ISR_PORT_C; *portPin = PINC2; break;
        case 26: *port = EVM_ISR_PORT_C; *portPin = PINC3; break;
        case 27: *port = EVM_ISR_PORT_C; *portPin = PINC4; break;
        case 28: *port = EVM_ISR_PORT_C; *portPin = PINC5; break;
        
        default: retVal = 0;
    }
    
    return retVal;
}




// ISR for Port B
ISR(PCINT0_vect)
{
    Ivm::OnISR(EVM_ISR_PORT_B, PINB, port__pinStateLast[EVM_ISR_PORT_B]);
    
    // Store current state for next time
    port__pinStateLast[EVM_ISR_PORT_B] = PINB;
}

// ISR for Port C
ISR(PCINT1_vect)
{
    Ivm::OnISR(EVM_ISR_PORT_C, PINC, port__pinStateLast[EVM_ISR_PORT_C]);
    
    // Store current state for next time
    port__pinStateLast[EVM_ISR_PORT_C] = PINC;
}

// ISR for Port D
ISR(PCINT2_vect)
{
    Ivm::OnISR(EVM_ISR_PORT_D, PIND, port__pinStateLast[EVM_ISR_PORT_D]);
    
    // Store current state for next time
    port__pinStateLast[EVM_ISR_PORT_D] = PIND;
}







#endif  // __AVR_ATmega328P__



















