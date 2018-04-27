#ifndef __WSPR_ENCODER_H__
#define __WSPR_ENCODER_H__


// Based on K6HX (Mark VandeWettering) implementation
// https://github.com/brainwagon/genwspr


#include <stdio.h>
#include <ctype.h>
#include <string.h>
//#include "UtlStreamBlob.h"


class WSPREncoder
{
public:


static int 
chval1(int ch)
{
    if (isdigit(ch)) return ch - '0' ;
    if (isalpha(ch)) return 10 + toupper(ch) - 'A' ;
    if (ch == ' ') return 36 ;
    
    return 0;
}

static int 
chval2(int ch)
{
    if (isalpha(ch)) return toupper(ch) - 'A' ;
    if (ch == ' ') return 26 ;
    
    return 0;
}


static uint32_t
encodecallsign(const char *callsign)
{
    /* find the first digit... */
    uint32_t i, rc ;
    char call[6] ;

    for (i=0; i<6; i++) call[i] = ' ' ;

    if (isdigit(callsign[1])) {
	/* 1x callsigns... */
	for (i=0; i<strlen(callsign); i++)
	   call[1+i] = callsign[i] ;
    } else if (isdigit(callsign[2])) {
	/* 2x callsigns... */
	for (i=0; i<strlen(callsign); i++)
	   call[i] = callsign[i] ;
    } else {
	return 0 ;
    }

    rc  = chval1(call[0]) ; rc *= 36 ; 
    rc += chval1(call[1]) ; rc *= 10 ;
    rc += chval1(call[2]) ; rc *= 27 ;
    rc += chval2(call[3]) ; rc *= 27 ;
    rc += chval2(call[4]) ; rc *= 27 ;
    rc += chval2(call[5]) ;
    
    // Serial.print("Callsign: "); Serial.println(rc);
    // StreamBlob(Serial, (uint8_t *)&rc, 4, 1, 1);

    return rc ;
}

static int 
encodegrid(const char *grid)
{
    int rc ;

    rc = (179 - 10 * (grid[0]-'A') - (grid[2] - '0')) * 180
	 + (10 * (grid[1]-'A')) + (grid[3] - '0') ;

    return rc ;
}

static int
encodepower(int p)
{
    return p + 64 ;
}

static int32_t
parity(uint32_t x)
{
    int32_t even = 0 ;
    while (x) {
	even = 1-even ;
	x = x & (x - 1) ;
    }
    return even ;
}



static void
genmsg(const char *call, const char *grid, const int power)
{
    uint32_t c = encodecallsign(call) ;
    uint32_t g = encodegrid(grid) ;
    uint32_t p = encodepower(power) ;
    int32_t i, mp = 0 ;
    uint32_t acc = 0;
    
    // uint32_t M = (g * 128) + p;
    // Serial.print("M: "); Serial.println(M);
    // StreamBlob(Serial, (uint8_t *)&M, 4, 1, 1);

    for (i=0; i<162; i++)
	msg[i] = sync[i] ;

    for (i=27; i>=0; i--) {		/* encode the callsign, 28 bits */
	acc <<= 1 ;
	if (c & (1L<<i)) acc |= 1 ;
    
    // StreamBlob(Serial, (uint8_t *)&acc, 4, 1, 1);
    
	msg[rdx[mp++]] += 2*parity(acc & 0xf2d05351L) ;
	msg[rdx[mp++]] += 2*parity(acc & 0xe4613c47L) ;
    }

    for (i=14; i>=0; i--) {		/* encode the grid, 15 bits */
	acc <<= 1 ;
	if (g & (1L<<i)) acc |= 1 ;
	msg[rdx[mp++]] += 2*parity(acc & 0xf2d05351L) ;
	msg[rdx[mp++]] += 2*parity(acc & 0xe4613c47L) ;
    }

    for (i=6; i>=0; i--) {		/* encode the power, 7 bits */
	acc <<= 1 ;
	if (p & (1L<<i)) acc |= 1 ;
	msg[rdx[mp++]] += 2*parity(acc & 0xf2d05351L) ;
	msg[rdx[mp++]] += 2*parity(acc & 0xe4613c47L) ;
    }

    for (i=30; i>=0; i--) {		/* pad with 31 zero bits */
	acc <<= 1 ;
	msg[rdx[mp++]] += 2*parity(acc & 0xf2d05351L) ;
	msg[rdx[mp++]] += 2*parity(acc & 0xe4613c47L) ;
    }
}

static const unsigned char sync[162];

static const unsigned char rdx[162];

static char msg[162];
};

const unsigned char WSPREncoder::sync[162] = {
    1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0,
    1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1,
    0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0,
    1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1,
    1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1,
    0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1,
    1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0
} ;

const unsigned char WSPREncoder::rdx[162] = {
    0, 128, 64, 32, 160, 96, 16, 144, 80, 48, 112, 8, 136, 72, 40, 104, 24,
    152, 88, 56, 120, 4, 132, 68, 36, 100, 20, 148, 84, 52, 116, 12, 140,
    76, 44, 108, 28, 156, 92, 60, 124, 2, 130, 66, 34, 98, 18, 146, 82, 50,
    114, 10, 138, 74, 42, 106, 26, 154, 90, 58, 122, 6, 134, 70, 38, 102,
    22, 150, 86, 54, 118, 14, 142, 78, 46, 110, 30, 158, 94, 62, 126, 1,
    129, 65, 33, 161, 97, 17, 145, 81, 49, 113, 9, 137, 73, 41, 105, 25,
    153, 89, 57, 121, 5, 133, 69, 37, 101, 21, 149, 85, 53, 117, 13, 141,
    77, 45, 109, 29, 157, 93, 61, 125, 3, 131, 67, 35, 99, 19, 147, 83, 51,
    115, 11, 139, 75, 43, 107, 27, 155, 91, 59, 123, 7, 135, 71, 39, 103,
    23, 151, 87, 55, 119, 15, 143, 79, 47, 111, 31, 159, 95, 63, 127 
} ;

char WSPREncoder::msg[162];



#endif  // __WSPR_ENCODER_H__


















