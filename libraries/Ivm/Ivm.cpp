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
    uint8_t retVal = 0;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if (pcieh->GetMode() != PCIntEventHandler::MODE::MODE_UNDEFINED)
        {
            retVal = 1;
            
            // Be sure there is a handler already active
            if (InterruptIsActiveForPhysicalPin(pcieh->GetPin()))
            {
                retVal = DetachInterruptForPhysicalPin(pcieh->GetPin());
            }
        }
        else
        {
            retVal = 0;
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
            pcieh->SetEventTimeUs(GetEventTimeUs());
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
// Unrolled for speed.
// On 8MHz, entire ISR (from ISR() start to finish was reduced from 50us to 23us)
void
Ivm::OnISR(uint8_t port,
           uint8_t bitmapPortPinState,
           uint8_t bitmapPortPinStateLast)
{
    // Calculate what changed, and how
    uint8_t bitmapChange    = (uint8_t)bitmapPortPinStateLast ^ bitmapPortPinState;
    uint8_t bitmapChangeDir = (uint8_t)bitmapPortPinState     & bitmapChange;

    {
        const uint8_t pin = 0;

        uint8_t pinChanged = (uint8_t)(bitmapChange & (uint8_t)_BV(pin));
        
        if (pinChanged)
        {
            // Normalize -- Change up == 1, Change down == 0
            uint8_t changeDir =
                (uint8_t)(bitmapChangeDir & (uint8_t)_BV(pin)) ? 1 : 0;
            
            Ivm::OnPortPinStateChange(port, pin, changeDir);
        }
    }

    {
        const uint8_t pin = 1;

        uint8_t pinChanged = (uint8_t)(bitmapChange & (uint8_t)_BV(pin));
        
        if (pinChanged)
        {
            // Normalize -- Change up == 1, Change down == 0
            uint8_t changeDir =
                (uint8_t)(bitmapChangeDir & (uint8_t)_BV(pin)) ? 1 : 0;
            
            Ivm::OnPortPinStateChange(port, pin, changeDir);
        }
    }

    {
        const uint8_t pin = 2;

        uint8_t pinChanged = (uint8_t)(bitmapChange & (uint8_t)_BV(pin));
        
        if (pinChanged)
        {
            // Normalize -- Change up == 1, Change down == 0
            uint8_t changeDir =
                (uint8_t)(bitmapChangeDir & (uint8_t)_BV(pin)) ? 1 : 0;
            
            Ivm::OnPortPinStateChange(port, pin, changeDir);
        }
    }

    {
        const uint8_t pin = 3;

        uint8_t pinChanged = (uint8_t)(bitmapChange & (uint8_t)_BV(pin));
        
        if (pinChanged)
        {
            // Normalize -- Change up == 1, Change down == 0
            uint8_t changeDir =
                (uint8_t)(bitmapChangeDir & (uint8_t)_BV(pin)) ? 1 : 0;
            
            Ivm::OnPortPinStateChange(port, pin, changeDir);
        }
    }

    {
        const uint8_t pin = 4;

        uint8_t pinChanged = (uint8_t)(bitmapChange & (uint8_t)_BV(pin));
        
        if (pinChanged)
        {
            // Normalize -- Change up == 1, Change down == 0
            uint8_t changeDir =
                (uint8_t)(bitmapChangeDir & (uint8_t)_BV(pin)) ? 1 : 0;
            
            Ivm::OnPortPinStateChange(port, pin, changeDir);
        }
    }

    {
        const uint8_t pin = 5;

        uint8_t pinChanged = (uint8_t)(bitmapChange & (uint8_t)_BV(pin));
        
        if (pinChanged)
        {
            // Normalize -- Change up == 1, Change down == 0
            uint8_t changeDir =
                (uint8_t)(bitmapChangeDir & (uint8_t)_BV(pin)) ? 1 : 0;
            
            Ivm::OnPortPinStateChange(port, pin, changeDir);
        }
    }

    {
        const uint8_t pin = 6;

        uint8_t pinChanged = (uint8_t)(bitmapChange & (uint8_t)_BV(pin));
        
        if (pinChanged)
        {
            // Normalize -- Change up == 1, Change down == 0
            uint8_t changeDir =
                (uint8_t)(bitmapChangeDir & (uint8_t)_BV(pin)) ? 1 : 0;
            
            Ivm::OnPortPinStateChange(port, pin, changeDir);
        }
    }

    {
        const uint8_t pin = 7;

        uint8_t pinChanged = (uint8_t)(bitmapChange & (uint8_t)_BV(pin));
        
        if (pinChanged)
        {
            // Normalize -- Change up == 1, Change down == 0
            uint8_t changeDir =
                (uint8_t)(bitmapChangeDir & (uint8_t)_BV(pin)) ? 1 : 0;
            
            Ivm::OnPortPinStateChange(port, pin, changeDir);
        }
    }
}

 
uint8_t
Ivm::RegisterBADISREventHandler(BADISREventHandler *beh)
{
    uint8_t retVal = 0;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        BADISREventHandler *behCurrent = GetBADISREventHandler();
        
        if (beh)
        {
            if (behCurrent != NULL || behCurrent == beh)
            {
                retVal = 1;
                
                AttachBADISREventHandler(beh);
            }
        }
    }
    
    return retVal;
}
 
 
uint8_t
Ivm::DeRegisterBADISREventHandler(BADISREventHandler *beh)
{
    uint8_t retVal = 0;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        BADISREventHandler *behCurrent = GetBADISREventHandler();
        
        if (beh == behCurrent)
        {
            retVal = 1;
            
            DetachBADISREventHandler();
        }
    }
    
    return retVal;
}
 
/*
 * Static function.
 *
 * Single function called from only the BADISR ISR.
 *
 * Does not try to hand off event to Evm or anything else.  All code paths
 * are executed within an ISR.
 *
 * This function effectively signals doomsday so feel free to clobber whatever.
 *
 */
void
Ivm::OnBADISR()
{
    BADISREventHandler *beh = Ivm::GetBADISREventHandler();
    
    if (beh)
    {
        beh->OnBADISREvent();
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
 */
 
 
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

// Lookup tables for access to port-specific pin values
// in an abstracted way
static volatile uint8_t  port__pinStateLast[3] = { 0,     0,     0     };

// Storage for BADISREventHandler
static BADISREventHandler *behPtr = NULL;

// Storage for timestamp of ISR fire time
static volatile uint32_t TIME_US_ISR = 0;


//////////////////////////////////////////////////////////////////////
//
// API implementation for this architecture
//
//////////////////////////////////////////////////////////////////////

uint32_t
Ivm::GetEventTimeUs()
{
    return TIME_US_ISR;
}

uint8_t
Ivm::AttachInterruptForPhysicalPin(uint8_t            physicalPin,
                                   PCIntEventHandler *pcieh)
{
    uint8_t retVal = 0;
    
    uint8_t port;
    uint8_t portPin;
    
    // Find the port and portPin to register handler on
    if (PAL.GetPortAndPortPinFromPhysicalPin(physicalPin, &port, &portPin))
    {
        retVal = 1;
        
        // Store this handler in the lookup table
        port_portPin__pcieh[port][portPin] = pcieh;
        
        // Default to explicitly setting this pin to be an input
        PAL.PinMode(physicalPin, INPUT);
        
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
            (( port__pinStateLast[port]                     & (uint8_t)~_BV(portPin) )  | // zero out bit
             ( PAL.GetPortValueFromPhysicalPin(physicalPin) & (uint8_t) _BV(portPin) ));  // combine live bit
        
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
    if (PAL.GetPortAndPortPinFromPhysicalPin(physicalPin, &port, &portPin))
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
    
    if (PAL.GetPortAndPortPinFromPhysicalPin(physicalPin, &port, &portPin))
    {
        retVal = (GetPCIntEventHandlerByPortAndPortPin(port, portPin) != NULL);
    }
    
    return retVal;
}

uint8_t
Ivm::AttachBADISREventHandler(BADISREventHandler *beh)
{
    uint8_t retVal = 0;
    
    if (beh && !behPtr)
    {
        behPtr = beh;
    }
    
    return retVal;
}

uint8_t
Ivm::DetachBADISREventHandler()
{
    uint8_t retVal = 0;
    
    if (behPtr)
    {
        retVal = 1;
        
        behPtr = NULL;
    }
    
    return retVal;
}

BADISREventHandler *
Ivm::GetBADISREventHandler()
{
    return behPtr;
}


// The actual ISRs
// Arduino doesn't support actual preprocessor directives, so this horrid
// hack is required.
#include "Ivm_conditional.h"



#endif  // __AVR_ATmega328P__





        














