#include "PAL.h"


//////////////////////////////////////////////////////////////////////
//
// Local Storage
//
//////////////////////////////////////////////////////////////////////

#ifdef __AVR_ATmega328P__

/*
 * Meant as a mapping for pins for IO.
 */
static int8_t physicalPin__arduinoPin[] = {
/* Physical | Arduino | Alias */
    
    /*  0 */  -1,   /* DNE  */
    /*  1 */  -1,
    /*  2 */   0,
    /*  3 */   1,
    /*  4 */   2,
    /*  5 */   3,
    /*  6 */   4,
    /*  7 */  -1,   /* VCC  */
    /*  8 */  -1,   /* GND  */
    /*  9 */  -1,
    /* 10 */  -1,
    /* 11 */   5,
    /* 12 */   6,
    /* 13 */   7,
    /* 14 */   8,
    /* 15 */   9,
    /* 16 */  10,   /* SS   */
    /* 17 */  11,   /* MOSI */
    /* 18 */  12,   /* MISO */
    /* 19 */  13,   /* SCK  */
    /* 20 */  -1,   /* AVCC */
    /* 21 */  -1,   /* AREF */
    /* 22 */  -1,   /* GND  */
    /* 23 */  14,   /* A0   */
    /* 24 */  15,   /* A1   */
    /* 25 */  16,   /* A2   */
    /* 26 */  17,   /* A3   */
    /* 27 */  18,   /* A4   */ /* SDA */
    /* 28 */  19    /* A5   */ /* SDL */
};


int8_t
PlatformAbstractionLayer::GetArduinoPinFromPhysicalPin(uint8_t physicalPin)
{
    return physicalPin__arduinoPin[physicalPin];
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



