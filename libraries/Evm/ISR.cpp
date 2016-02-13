#include <avr/interrupt.h>

#include "ISR.h"
#include "Evm.h"


/*
 * Main Principle of Interrupt Synchronization Architecture:
 * - ISR-invoked code can only add to Evm InterruptEventHandler list
 *   - and never enables interrupts while running
 *
 * - Evm-invoked code can read/write the InterruptEventHandler list
 *   - but does so only in protected code which prevents ISRs from firing
 *     - and takes care to not rely on the structure being unchanged outside
 *       of protected areas.
 *
 * - This means most complexity still lies in main thread code, ISRs simply
 *   queue data for Evm to deal with later.
 *
 * - Additionally this means there can only be 1 of 2 types of code running
 *   while accessing data which can be reached from both ISR and Evm.
 *
 * - Lastly this makes ISR-invoked code return very quickly, maximizing
 *   opportunity for other ISRs to be serviced, since they are blocked out
 *   while other ISRs are running.
 *
 *
 *
 *
 * Main Principle of Architecture-specific implementation of Interrupt Handling
 * - Establish an API that any given architecture can implement.
 * - Make use of that API within the externally-visible code.
 * - Detect (via #ifdef) a particular architecture, and implement.
 *
 */

 
 

 
//////////////////////////////////////////////////////////////////////
//
// Forward Declarations for API provided
// by Architecture-specific implementations
//
//////////////////////////////////////////////////////////////////////


static void
ISR_AttachInterruptForPhysicalPin(uint8_t                physicalPin,
                                  InterruptEventHandler *ieh);
static void
ISR_DetachInterruptForPhysicalPin(uint8_t physicalPin);

static uint8_t
ISR_InterruptIsActiveForPhysicalPin(uint8_t physicalPin);
 

 
 
 
 
 
 
 

//////////////////////////////////////////////////////////////////////
//
// Externally visible functions
//
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
// These functions are only called from "Main Thread" code.
//
// They require protection from reentrant effects since they access
// and lead to data structures which are also accessible from ISRs.
//
//////////////////////////////////////////////////////////////////////

void ISR_RegisterForInterruptEvent(InterruptEventHandler *ieh)
{
    cli();
    
    // Don't allow double registration
    if (!ISR_InterruptIsActiveForPhysicalPin(ieh->GetPin()))
    {
        ISR_AttachInterruptForPhysicalPin(ieh->GetPin(), ieh);
    }
    
    sei();
}

void ISR_DeRegisterForInterruptEvent(InterruptEventHandler *ieh)
{
    cli();
    
    // Be sure there is a handler already active
    if (ISR_InterruptIsActiveForPhysicalPin(ieh->GetPin()))
    {
        ISR_DetachInterruptForPhysicalPin(ieh->GetPin());
        
        // Dequeue any timed events already scheduled
        Evm::GetInstance().DeRegisterInterruptEventHandler(ieh);
    }
    
    sei();
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
 
 
 

//////////////////////////////////////////////////////////////////////
//
// Local Definitions
//
//////////////////////////////////////////////////////////////////////
 
#define EVM_ISR_PORT_B 0
#define EVM_ISR_PORT_C 1
#define EVM_ISR_PORT_D 2


//////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
//////////////////////////////////////////////////////////////////////

static uint8_t
ISR_GetPortAndPortPinFromPhysicalPin(uint8_t  physicalPin,
                                     uint8_t *port,
                                     uint8_t *portPin);
 

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
static InterruptEventHandler *port_portPin__ieh[3][8] = { NULL };

// Lookup tables for access to port-specific data in an abstracted way
static          uint8_t  port__refCount[3] = { 0 };
static volatile uint8_t *port__pcmskPtr[3] = { &PCMSK0, &PCMSK1, &PCMSK2 };
static volatile uint8_t *port__ddrPtr[3]   = { &DDRB,   &DDRC,   &DDRD   };

// Storage to keep track of the last state of each port's pins
static uint8_t pinStateLast[3] = { 0 };


//////////////////////////////////////////////////////////////////////
//
// API implementation for this architecture
//
//////////////////////////////////////////////////////////////////////

static void
ISR_AttachInterruptForPhysicalPin(uint8_t                physicalPin,
                                  InterruptEventHandler *ieh)
{
    uint8_t port;
    uint8_t portPin;
    
    // Find the port and portPin to register handler on
    if (ISR_GetPortAndPortPinFromPhysicalPin(physicalPin, &port, &portPin))
    {
        // Store this handler in the lookup table
        port_portPin__ieh[port][portPin] = ieh;
        
        // Default to explicitly setting this pin to be an output
        *port__ddrPtr[port] |= _BV(portPin);
        
        // Enable interrupts for this specific pin on this port
        *port__pcmskPtr[port] |= _BV(portPin);
        
        // If there were previously no interrupts registered for this port
        // then enable interrupts for this port
        if (!port__refCount[port])
        {
            PCICR |= _BV(port);
        }
        
        // Increment reference count for users of this port
        ++port__refCount[port];
    }
}

static void
ISR_DetachInterruptForPhysicalPin(uint8_t physicalPin)
{
    uint8_t port;
    uint8_t portPin;
    
    // Find the port and portPin to register handler on
    if (ISR_GetPortAndPortPinFromPhysicalPin(physicalPin, &port, &portPin))
    {
        // Remove the handler from the lookup table
        port_portPin__ieh[port][portPin] = NULL;
        
        // Change nothing about the output state of the pin
        
        // Disable interrupts for this specific pon on this port
        *port__pcmskPtr[port] &= ~_BV(portPin);
        
        // Decrement reference count for users of this port
        --port__refCount[port];
        
        // If there are now no interrupts registered for this port
        // then disable interrupts for this port
        if (!port__refCount[port])
        {
            PCICR &= ~_BV(port);
        }
    }
}

static uint8_t
ISR_InterruptIsActiveForPhysicalPin(uint8_t physicalPin)
{
    uint8_t retVal = 0;
    
    uint8_t port;
    uint8_t portPin;
    
    if (ISR_GetPortAndPortPinFromPhysicalPin(physicalPin, &port, &portPin))
    {
        retVal = (port_portPin__ieh[port][portPin] != NULL);
    }
    
    return retVal;
}


 

 
 
//////////////////////////////////////////////////////////////////////
//
// Private implementation details
//
//////////////////////////////////////////////////////////////////////

// Return 1 for successfully found, 0 for failure
static uint8_t
ISR_GetPortAndPortPinFromPhysicalPin(uint8_t  physicalPin,
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


//////////////////////////////////////////////////////////////////////
//
// Actual Interrupt vector handling.
//
// These functions are only called from an ISR.
//
// As a result, no protection from reentrant effects is required.
//
//////////////////////////////////////////////////////////////////////

static void
ISR_OnPortPinStateChange(uint8_t port, uint8_t portPin, uint8_t changeDir)
{
    InterruptEventHandler *ieh = port_portPin__ieh[port][portPin];
    
    InterruptEventHandler::MODE changeType = (
        changeDir                                 ?
        InterruptEventHandler::MODE::MODE_RISING  :
        InterruptEventHandler::MODE::MODE_FALLING
    );
    
    if (ieh)
    {
        if (ieh->GetMode() == changeType)
        {
            Evm::GetInstance().RegisterInterruptEventHandler(ieh);
        }
    }
}


/*
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
static void
ISR_OnISR(uint8_t port,
          uint8_t bitmapPortPinState,
          uint8_t bitmapPortPinStateLast)
{
    // Calculate what changed, and how
    uint8_t bitmapChange    = bitmapPortPinStateLast ^ bitmapPortPinState;
    uint8_t bitmapChangeDir = bitmapPortPinState     & bitmapChange;

    // Apply bitmaps to figure out which pins actually changed and how
    for (uint8_t portPin = 0; portPin < 8; ++portPin)
    {
        uint8_t pinChanged = (bitmapChange & _BV(portPin));
        
        if (pinChanged)
        {
            // Normalize -- Change up == 1, Change down == 0
            uint8_t changeDir = (bitmapChangeDir & _BV(portPin)) ? 1 : 0;
            
            ISR_OnPortPinStateChange(port, portPin, changeDir);
        }
    }
}



// ISR for Port B
ISR(PCINT0_vect)
{
    ISR_OnISR(EVM_ISR_PORT_B, PINB, pinStateLast[EVM_ISR_PORT_B]);
    
    // Store current state for next time
    pinStateLast[EVM_ISR_PORT_B] = PINB;
}

// ISR for Port C
ISR(PCINT1_vect)
{
    ISR_OnISR(EVM_ISR_PORT_C, PINC, pinStateLast[EVM_ISR_PORT_C]);
    
    // Store current state for next time
    pinStateLast[EVM_ISR_PORT_C] = PINC;
}

// ISR for Port D
ISR(PCINT2_vect)
{
    ISR_OnISR(EVM_ISR_PORT_D, PIND, pinStateLast[EVM_ISR_PORT_D]);
    
    // Store current state for next time
    pinStateLast[EVM_ISR_PORT_D] = PIND;
}


#endif  // __AVR_ATmega328P__



















