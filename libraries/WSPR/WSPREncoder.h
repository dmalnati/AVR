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

#include "CString.h"
#include "PAL.h"
#include "BitField.h"
#include "nhash.h"


/*
 * This library is mixing the simplicity of the genwspr code with my own
 * memory elimination techniques, as well as applying a generation approach to
 * the distribution from JTEncode.
 *
 */

class WSPREncoder
{
    static const uint8_t CALLSIGN_LEN = 6;
    static const uint8_t GRID_LEN     = 4;
    

////////////////////////////////////////////////////////////////////////////////
//
// Type 1 Messages
//
////////////////////////////////////////////////////////////////////////////////

public:

static void
EncodeType1(const char *callsignInput,
            const char *gridInput,
            uint8_t     powerDbmInput)
{
    genmsg_type1(callsignInput, gridInput, powerDbmInput);
}

static void
genmsg_type1(const char *call, const char *grid, const int power)
{
    uint32_t c = encodecallsign(call) ; // 28 bits
    uint32_t g = encodegrid(grid) ;     // 15 bits
    uint32_t p = encodepower(power) ;   //  7 bits
    
    // Convert the useful bits above into a bitfield for further processing
    BitFieldOwned<50> bf;
    uint8_t idx = 0;
    
    bf.SetBitRangeAt(idx, c, 28);
    idx += 28;
    
    bf.SetBitRangeAt(idx, g, 15);
    idx += 15;
    
    bf.SetBitRangeAt(idx, p, 7);
    
    EncodeTones(bf);
}

private:


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

//
// http://hojoham.blogspot.com/2016/09/hojo-and-case-of-garbled-wspr-packet.html
// fix applied
//
static uint32_t
encodecallsign(const char *callsign)
{
    /* find the first digit... */
    uint32_t i, rc ;
    char call[6] ;

    for (i=0; i<6; i++) call[i] = ' ' ;

    if (isdigit(callsign[1]) && !isdigit(callsign[2]))
    {
        /* 1x callsigns... */
        for (i=0; i<strlen(callsign); i++)
           call[1+i] = callsign[i] ;
    }
    else if (isdigit(callsign[2]))
    {
        /* 2x callsigns... */
        for (i=0; i<strlen(callsign); i++)
           call[i] = callsign[i] ;
    }
    else
    {
        return 0 ;
    }

    rc  = chval1(call[0]) ; rc *= 36 ; 
    rc += chval1(call[1]) ; rc *= 10 ;
    rc += chval1(call[2]) ; rc *= 27 ;
    rc += chval2(call[3]) ; rc *= 27 ;
    rc += chval2(call[4]) ; rc *= 27 ;
    rc += chval2(call[5]) ;
    
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


////////////////////////////////////////////////////////////////////////////////
//
// Tone Extraction
//
////////////////////////////////////////////////////////////////////////////////

public:

static uint8_t GetToneValForSymbol(uint8_t idx)
{
    uint16_t bitIdx = idx * 2;
    
    uint8_t val;
    bfToneNumList_.GetBitRangeAt(bitIdx, val, 2);
    
    return val;
}

private:


////////////////////////////////////////////////////////////////////////////////
//
// Tone Generation
//
////////////////////////////////////////////////////////////////////////////////

// Should have 50 bits of data in the buffer.
// This function will encode them into the final transmittable tone value form.
//
// We are replicating the encoding scheme seen in the genmsg function originally.
//
// eg
//    uint32_t c = encodecallsign(call) ;
//     ...
//    for (i=27; i>=0; i--) /* encode the callsign, 28 bits */
//    {
//	      acc <<= 1 ;
//	      if (c & (1L<<i)) acc |= 1 ;
//    
//        IncrToneVal(GetRdx(mp++), 2*parity(acc & 0xf2d05351L));
//        IncrToneVal(GetRdx(mp++), 2*parity(acc & 0xe4613c47L));
//    }
//
// 
// Seeing that we're striping left-to-right and extracting bits from the input.
// When that bit is set, we set the right-most bit on the accumulator.
//   The accumulator is always shifting left.
//
// So basically the bits in the accumulator are a mirror-image of the incoming
// bit-stream within a rolling 32-bit window.
//
static void
EncodeTones(BitField &bitFieldOf50Bits)
{
    uint32_t acc     = 0;
    uint8_t  idxTone = 0;
    
    // Lay down the initial tone values from synchronization bits
    for (uint8_t i = 0; i < 162; ++i)
    {
        uint8_t val;
        bfSync_.GetBitAt(i, val);
        
        SetToneVal(i, val);
    }
    
    // Generate the first 100 tone values from the input 50 bits
    for (uint8_t i = 0; i < 50; ++i)
    {
        acc <<= 1;
        
        uint8_t bitVal;
        bitFieldOf50Bits.GetBitAt(i, bitVal);
        
        if (bitVal)
        {
            acc |= 1;
        }
        
        IncrToneVal(GetRdx(idxTone++), 2 * parity(acc & 0xf2d05351L));
        IncrToneVal(GetRdx(idxTone++), 2 * parity(acc & 0xe4613c47L));
    }
    
    // Generate the remaining 62 tone values from zeroes
    for (uint8_t i = 0; i < 31; ++i)
    {
        acc <<= 1;
        
        IncrToneVal(GetRdx(idxTone++), 2 * parity(acc & 0xf2d05351L));
        IncrToneVal(GetRdx(idxTone++), 2 * parity(acc & 0xe4613c47L));
    }
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





private:

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


















