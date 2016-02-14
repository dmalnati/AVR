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

#endif  // __AVR_ATmega328P__



// Globally-available instance.
PlatformAbstractionLayer PAL;



