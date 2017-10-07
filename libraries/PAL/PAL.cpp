#include "PAL.h"


//////////////////////////////////////////////////////////////////////
//
// Local Storage
//
//////////////////////////////////////////////////////////////////////

#ifdef __AVR_ATmega328P__


int8_t
PlatformAbstractionLayer::GetArduinoPinFromPhysicalPin(uint8_t physicalPin)
{
    int8_t retVal = -1;
    
    /* Physical | Arduino | Alias */

    if      (physicalPin ==  0) { retVal = -1; }   /* DNE  */
    else if (physicalPin ==  1) { retVal = -1; }
    else if (physicalPin ==  2) { retVal =  0; }
    else if (physicalPin ==  3) { retVal =  1; }
    else if (physicalPin ==  4) { retVal =  2; }
    else if (physicalPin ==  5) { retVal =  3; }
    else if (physicalPin ==  6) { retVal =  4; }
    else if (physicalPin ==  7) { retVal = -1; }   /* VCC  */
    else if (physicalPin ==  8) { retVal = -1; }   /* GND  */
    else if (physicalPin ==  9) { retVal = -1; }
    else if (physicalPin == 10) { retVal = -1; }
    else if (physicalPin == 11) { retVal =  5; }
    else if (physicalPin == 12) { retVal =  6; }
    else if (physicalPin == 13) { retVal =  7; }
    else if (physicalPin == 14) { retVal =  8; }
    else if (physicalPin == 15) { retVal =  9; }
    else if (physicalPin == 16) { retVal = 10; }   /* SS   */
    else if (physicalPin == 17) { retVal = 11; }   /* MOSI */
    else if (physicalPin == 18) { retVal = 12; }   /* MISO */
    else if (physicalPin == 19) { retVal = 13; }   /* SCK  */
    else if (physicalPin == 20) { retVal = -1; }   /* AVCC */
    else if (physicalPin == 21) { retVal = -1; }   /* AREF */
    else if (physicalPin == 22) { retVal = -1; }   /* GND  */
    else if (physicalPin == 23) { retVal = 14; }   /* A0   */
    else if (physicalPin == 24) { retVal = 15; }   /* A1   */
    else if (physicalPin == 25) { retVal = 16; }   /* A2   */
    else if (physicalPin == 26) { retVal = 17; }   /* A3   */
    else if (physicalPin == 27) { retVal = 18; }   /* A4   */ /* SDA */
    else if (physicalPin == 28) { retVal = 19; }   /* A5   */ /* SDL */
    
    return retVal;
}




// Return 1 for successfully found, 0 for failure
// 'port' is really an opaque handle, not the actual port.
// same with portPin.
uint8_t
PlatformAbstractionLayer::GetPortAndPortPinFromPhysicalPin(uint8_t  physicalPin,
                                                           uint8_t *port,
                                                           uint8_t *portPin)
{
    uint8_t retVal = 1;
    
    if      (physicalPin ==  0) { retVal = 0;                       }
    else if (physicalPin ==  1) { *port = PORT_C; *portPin = PINC6; }
    else if (physicalPin ==  2) { *port = PORT_D; *portPin = PIND0; }
    else if (physicalPin ==  3) { *port = PORT_D; *portPin = PIND1; }
    else if (physicalPin ==  4) { *port = PORT_D; *portPin = PIND2; }
    else if (physicalPin ==  5) { *port = PORT_D; *portPin = PIND3; }
    else if (physicalPin ==  6) { *port = PORT_D; *portPin = PIND4; }
    else if (physicalPin ==  7) { retVal = 0;                       }
    else if (physicalPin ==  8) { retVal = 0;                       }
    else if (physicalPin ==  9) { *port = PORT_B; *portPin = PINB6; }
    else if (physicalPin == 10) { *port = PORT_B; *portPin = PINB7; }
    else if (physicalPin == 11) { *port = PORT_D; *portPin = PIND5; }
    else if (physicalPin == 12) { *port = PORT_D; *portPin = PIND6; }
    else if (physicalPin == 13) { *port = PORT_D; *portPin = PIND7; }
    else if (physicalPin == 14) { *port = PORT_B; *portPin = PINB0; }
    else if (physicalPin == 15) { *port = PORT_B; *portPin = PINB1; }
    else if (physicalPin == 16) { *port = PORT_B; *portPin = PINB2; }
    else if (physicalPin == 17) { *port = PORT_B; *portPin = PINB3; }
    else if (physicalPin == 18) { *port = PORT_B; *portPin = PINB4; }
    else if (physicalPin == 19) { *port = PORT_B; *portPin = PINB5; }
    else if (physicalPin == 20) { retVal = 0;                       }
    else if (physicalPin == 21) { retVal = 0;                       }
    else if (physicalPin == 22) { retVal = 0;                       }
    else if (physicalPin == 23) { *port = PORT_C; *portPin = PINC0; }
    else if (physicalPin == 24) { *port = PORT_C; *portPin = PINC1; }
    else if (physicalPin == 25) { *port = PORT_C; *portPin = PINC2; }
    else if (physicalPin == 26) { *port = PORT_C; *portPin = PINC3; }
    else if (physicalPin == 27) { *port = PORT_C; *portPin = PINC4; }
    else if (physicalPin == 28) { *port = PORT_C; *portPin = PINC5; }
    else                        { retVal = 0;                       }
    
    return retVal;
}


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





#endif  // __AVR_ATmega328P__



// Globally-available instance.
PlatformAbstractionLayer PAL;



