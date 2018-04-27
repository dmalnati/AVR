#ifndef __WSPR_ENCODER_H__
#define __WSPR_ENCODER_H__


// Based on K6HX (Mark VandeWettering) implementation
// https://github.com/brainwagon/genwspr
//
// As well as the JTEncode library
// https://github.com/etherkit/JTEncode


#include <stdio.h>
#include <ctype.h>
#include <string.h>
//#include "UtlStreamBlob.h"

#include "BitField.h"


/*
 * This library is mixing the simplicity of the genwspr code with my own
 * memory elimination techniques, as well as applying a generation approach to
 * the distribution from JTEncode.
 *
 * Original memory usage of raw genwspr lib
 * - prog 15688
 * - sram 1105
 * 
 * (after getting rid of sync bits)
 * - prog 15624
 * - sram 966
 * 
 * (after getting rid of msg)
 * - prog 16076
 * - sram 848
 * 
 * (after getting rid of rdx)
 * - prog 16180
 * - sram 686
 * 
 * total change
 * - prog +492
 * - sram -419
 *
 */

class WSPREncoder
{
public:

static uint8_t GetToneValForSymbol(uint8_t idx)
{
    uint16_t bitIdx = idx * 2;
    
    uint8_t val;
    bfToneNumList_.GetBitRangeAt(bitIdx, val, 2);
    
    return val;
}



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

    for (uint8_t i=0; i<162; i++)
    {
        uint8_t val;
        bfSync_.GetBitAt(i, val);
        
        SetToneVal(i, val);
    }

    for (i=27; i>=0; i--) {		/* encode the callsign, 28 bits */
	acc <<= 1 ;
	if (c & (1L<<i)) acc |= 1 ;
    
    // StreamBlob(Serial, (uint8_t *)&acc, 4, 1, 1);
    
    IncrToneVal(GetRdx(mp++), 2*parity(acc & 0xf2d05351L));
    IncrToneVal(GetRdx(mp++), 2*parity(acc & 0xe4613c47L));
    }

    for (i=14; i>=0; i--) {		/* encode the grid, 15 bits */
	acc <<= 1 ;
	if (g & (1L<<i)) acc |= 1 ;
    IncrToneVal(GetRdx(mp++), 2*parity(acc & 0xf2d05351L));
    IncrToneVal(GetRdx(mp++), 2*parity(acc & 0xe4613c47L));
    }

    for (i=6; i>=0; i--) {		/* encode the power, 7 bits */
	acc <<= 1 ;
	if (p & (1L<<i)) acc |= 1 ;
    IncrToneVal(GetRdx(mp++), 2*parity(acc & 0xf2d05351L));
    IncrToneVal(GetRdx(mp++), 2*parity(acc & 0xe4613c47L));
    }

    for (i=30; i>=0; i--) {		/* pad with 31 zero bits */
	acc <<= 1 ;
    IncrToneVal(GetRdx(mp++), 2*parity(acc & 0xf2d05351L));
    IncrToneVal(GetRdx(mp++), 2*parity(acc & 0xe4613c47L));
    }
}

// this (modified) function taken from JTEncode
static uint8_t GetRdx(uint8_t idx)
{
    static const uint8_t WSPR_BIT_COUNT = 162;
    
	uint8_t rev, index_temp, i, j, k;

	i = 0;

	for(j = 0; j < 255; j++)
	{
		// Bit reverse the index
		index_temp = j;
		rev = 0;

		for(k = 0; k < 8; k++)
		{
			if(index_temp & 0x01)
			{
				rev = rev | (1 << (7 - k));
			}
			index_temp = index_temp >> 1;
		}

		if(rev < WSPR_BIT_COUNT)
		{
            if (i == idx)
            {
                return rev;
            }
            else
            {
                i++;
            }
		}
	}
    
    return 0;
}



static void SetToneVal(uint8_t idx, uint8_t val)
{
    uint16_t bitIdx = idx * 2;
    
    bfToneNumList_.SetBitRangeAt(bitIdx, val, 2);
}

static void IncrToneVal(uint8_t idx, uint8_t val)
{
    uint16_t bitIdx = idx * 2;
    
    uint8_t valCurr;
    bfToneNumList_.GetBitRangeAt(bitIdx, valCurr, 2);
    
    valCurr += val;
    
    bfToneNumList_.SetBitRangeAt(bitIdx, valCurr, 2);
}

static const uint8_t syncBitList_[21];
static const BitField bfSync_;

static uint8_t toneNumListBuf_[41];
static BitField bfToneNumList_;
};

// need 162 bits, so pack into bytes
//
// 21 bytes * 8 bits = 168 (6 too many)
// so just add 6 bits of zero at the end
const uint8_t WSPREncoder::syncBitList_[21] = {
    0b11000000,
    0b10001110,
    0b00100101,
    0b11100000,
    0b00100101,
    0b00000010,
    0b11001101,
    0b00011010,
    0b00011010,
    0b10101001,
    0b00101100,
    0b01101010,
    0b00100000,
    0b10010011,
    0b10110011,
    0b01000111,
    0b00000101,
    0b00110000,
    0b00011010,
    0b11000110,
    0b00000000,
};

const BitField WSPREncoder::bfSync_((uint8_t *)WSPREncoder::syncBitList_, 21);


// each of the 162 data bits is represented in 4FSK
// meaning 4 possible values, so 2 bits to store the result.
//
// so need to store 162 * 2 = 324 bits
// 324 bits / 8 bits-per-byte = 40.5 bytes, so call it 41 bytes
uint8_t WSPREncoder::toneNumListBuf_[41];
BitField WSPREncoder::bfToneNumList_(WSPREncoder::toneNumListBuf_, 41);


#endif  // __WSPR_ENCODER_H__


















