#ifndef __WSPR_MESSAGE_PICO_TRACKER_WSPR_2_H__
#define __WSPR_MESSAGE_PICO_TRACKER_WSPR_2_H__


#include <stdint.h>
#include "WSPRMessage.h"


class WSPRMessagePicoTrackerWSPR2
: public WSPRMessage
{
public:

    // 2-char ID, representing the 1st and 3rd character of the callsign to
    // actually be transmitted over WSPR, which seems to be the convention out
    // in the world.
    void SetId(const char *id)
    {
        strncpy(id_, id, ID_LEN);

        Recalculate();
    }
    
    const char *GetId()
    {
        return id_;
    }

    // hides base-class SetGrid
    void SetGrid(const char *grid)
    {
        strncpy(grid_, grid, GRID_LEN);

        Recalculate();
    }
    
    const char *GetGrid()
    {
        return grid_;
    }
    
    void SetAltitudeFt(uint32_t altitudeFt)
    {
        altitudeFt_ = altitudeFt;
        
        Recalculate();
    }
    
    uint32_t GetAltitudeFt()
    {
        return altitudeFt_;
    }
    
    void SetSpeedKnots(uint8_t speedKnots)
    {
        speedKnots_ = speedKnots;
        
        Recalculate();
    }
    
    uint8_t GetSpeedKnots()
    {
        return speedKnots_;
    }
    
    void SetTemperatureC(int8_t temperatureC)
    {
        temperatureC_ = temperatureC;
        
        Recalculate();
    }
    
    int8_t GetTemperatureC()
    {
        return temperatureC_;
    }
    
    void SetMilliVoltage(uint16_t milliVolt)
    {
        milliVolt_ = milliVolt;
        
        Recalculate();
    }
    
    uint16_t GetMilliVoltage()
    {
        return milliVolt_;
    }

private:

    uint8_t Pack(uint8_t packTarget, uint8_t packVal, uint8_t valueCount)
    {
        uint8_t retVal = packTarget;
        
        retVal *= valueCount;
        retVal += packVal;
        
        return retVal;
    }

    char MapToAlpha(uint8_t val)
    {
        char retVal = 'Z';
        
        // We're given an integer number, 0-25, so 26 values.
        // It needs to be represented as an ASCII alphabetic character.
        
        if (val <= 25)
        {
            retVal = 'A' + val;
        }
        
        return retVal;
    }
    
    char MapToNum(uint8_t val)
    {
        char retVal = '9';
        
        // We're given an integer number, 0-9, so 10 values.
        // It needs to be represented as an ASCII numeric character.

        if (val <= 9)
        {
            retVal = '0' + val;
        }
        
        return retVal;
    }
    
    char MapToAlphaNum(uint8_t val)
    {
        char retVal;
        
        // We're given an integer number, 0-35, so 36 values.
        // It needs to be represented as an ASCII character.
        // Consider 0-9,A-Z to be the 36 values.
        // The first 10 values (0-9) go the number mapper.
        // Everything else goes to ASCII.
        //   However, the meaning of ASCII in this mode is different than
        //   standalone.
        //   In this mode, A represents the 11th value, which is value 10, so
        //   to make dual use of the alpha converter, we subtract 10 from the
        //   numeric value, and let alpha work as usual and ultimately get the
        //   right result.
        
        if (val <= 9)
        {
            retVal = MapToNum(val);
        }
        else
        {
            retVal = MapToAlpha(val - 10);
        }
        
        return retVal;
    }
    
    char MapToAlphaSpace(uint8_t val)
    {
        char retVal = ' ';
        
        // We're given an integer number, 0-26, so 27 values.
        // It needs to be represented as an ASCII character, if in the range
        // of 0-25.
        // If 25 or greater, use a space character.
        
        if (val <= 25)
        {
            retVal = MapToAlpha(val);
        }
        
        return retVal;
    }
    
    // General algorithm to iteratively walk up a ladder of increasing step size
    // in order to know what value it is greater-than-equal-to by knowing the
    // step it reached.
    // 
    // stepSize must be positive.
    // initialValue may be negative.
    static uint8_t GetIdxLastAtOrAbove(int32_t val,
                                       uint8_t idxMax,
                                       int32_t stepSize,
                                       int32_t initialValue = 0)
    {
        uint8_t idxLastAtOrAbove = 0;
        int32_t valToCompare     = initialValue;
        
        for (uint8_t i = 0; i < idxMax; ++i)
        {
            if (val >= valToCompare)
            {
                idxLastAtOrAbove = i;
            }
            
            valToCompare += stepSize;
        }
        
        return idxLastAtOrAbove;
    }
    
    static uint8_t CalculateSpeedValue(uint8_t speedKnots)
    {
        // Speed is in the range of 0-128 knots
        // It is in increments of 16 knots.
        // That makes 9 values.
        //
        // 0 * 16 =   0 knots
        // 1 * 16 =  16 knots
        // ...
        // 8 * 16 = 128 knots
        
        uint8_t idxLastAtOrAbove = GetIdxLastAtOrAbove(speedKnots, 9, 16);
        
        return idxLastAtOrAbove;
    }

    static void CalculateAltitudeValues(uint32_t  altitudeFt,
                                        uint8_t  &power,
                                        uint8_t  &ftIncr1000,
                                        uint8_t  &ftIncr500)
    {
        // The power value in WSPR is a non-calculatable list of acceptable
        // values.
        // The number of 2k ft increments is the index into this list
        constexpr uint8_t count2k__power[] = { 0, 3, 7, 10, 13, 17, 20, 23, 27, 30, 33, 37, 40, 43, 47, 50, 53, 57, 60, };
        
        // To determine the parameters passed in, we will
        // step by 500ft increments
        //
        // The count of the number of 500ft increments conveniently tells you
        // the altitude in bits of the count.
        //
        // eg:
        // 0b0000 = 2k * 0 + 1k * 0 + 500 * 0 =    0 ft
        // 0b0001 = 2k * 0 + 1k * 0 + 500 * 1 =  500 ft
        // 0b0010 = 2k * 0 + 1k * 1 + 500 * 0 = 1000 ft
        // 0b0011 = 2k * 0 + 1k * 1 + 500 * 1 = 1500 ft
        // 0b0100 = 2k * 1 + 1k * 0 + 500 * 0 = 2000 ft
        // 0b0101 = 2k * 1 + 1k * 0 + 500 * 1 = 2500 ft
        // ...
        // 
        // The total supported range is 0 ft - 37,500 ft
        // That is 76 possible values (37,500 / 500 = 75; + 1 for 0ft)
        // That is a max index of 75
        // 
        // 75 in binary is:
        // 0b01001011
        //
        // Decomposed, that is:
        // 010010 (18 * 2k    increments = 36,000 ft)
        //      1 ( 1 * 1k    increment  =  1,000 ft)
        //      1 ( 1 * 500ft increment  =    500 ft)
        //
        // That adds up to 37,500, which matches our expected max value.
        
        // Calculate it
        uint8_t idxLastAtOrAbove = GetIdxLastAtOrAbove(altitudeFt, 76, 500);
        
        // The number of 500 ft increments is the last bit, and shift out
        ftIncr500 = idxLastAtOrAbove & 0x01;
        idxLastAtOrAbove >>= 1;
        
        // The number of 1000 ft increments is the last bit, and shift out
        ftIncr1000 = idxLastAtOrAbove & 0x01;
        idxLastAtOrAbove >>= 1;
        
        // The number of 2000 ft increments is what remains
        uint8_t count2kIncrements = idxLastAtOrAbove;
        power = count2k__power[count2kIncrements];
    }
    
    static uint8_t CalculateTemperatureCValue(int8_t temperatureC)
    {
        // The temperature is in the range of -50 to 20 C
        // It is in increments of 10C.
        // That makes 8 values: -50, -40, -30, -20, -10, 0, 10, 20
        
        uint8_t idxLastAtOrAbove = GetIdxLastAtOrAbove(temperatureC, 8, 10, -50);
        
        return idxLastAtOrAbove;
    }

    static uint8_t CalculateMilliVoltValue(uint16_t milliVolt)
    {
        // The milliVolt is in the range of 1500 - 4500
        // It is in increments of 1500
        // That makes 3 values: 1500, 3000, 4500
        
        uint8_t idxLastAtOrAbove = GetIdxLastAtOrAbove(milliVolt, 3, 1500, 1500);
        
        return idxLastAtOrAbove;
    }

    void Recalculate()
    {
        // The interface of the base-class is:
        // - SetCallsign
        // - SetGrid
        // - SetPower
        //
        // We've cached all the logical values, now calculate the inputs to the
        // base-class and then invoke them.
        
        // Start by filling out the callsign buffer, which will be used for:
        //
        // Position    Values      Usage (number of values)
        // Callsign 1  Q,0         Telemetry Channel (2)
        // Callsign 2  0-9,A-Z     Speed (9)
        //                         Altitude 1,000ft increment (2)
        //                         Altitude 5000ft increment (2)
        // Callsign 3  0-9         Telemetry Channel (10)
        // Callsign 4  A-Z         Grid Square 5th char (A-X) (24)
        // Callsign 5  A-Z         Grid Square 6th char (A-X) (24)
        // Callsign 6  A-Z, space  Temperature(8)
        //                         Voltage(3)
        
        
        // Start by breaking down all provided inputs into the constituents
        // specified above.
        uint8_t speedKnotsValue = CalculateSpeedValue(speedKnots_);
        
        uint8_t power      = 0;
        uint8_t ftIncr1000 = 0;
        uint8_t ftIncr500  = 0;
        CalculateAltitudeValues(altitudeFt_, power, ftIncr1000, ftIncr500);
        
        uint8_t temperatureCValue = CalculateTemperatureCValue(temperatureC_);
        uint8_t milliVoltValue    = CalculateMilliVoltValue(milliVolt_);
        
        
        // Temporary storage
        const uint8_t CALLSIGN_LEN = 6;
        char callsign[CALLSIGN_LEN + 1] = { 0 };
        
        
        // Callsign 1 is the first part of the ID
        callsign[0] = id_[0];
        
        // Callsign 2 is Speed, Altitude 1000ft, Altitude 500ft
        uint8_t cs2Val = 0;
        cs2Val = Pack(cs2Val, speedKnotsValue, 9);
        cs2Val = Pack(cs2Val, ftIncr1000,      2);
        cs2Val = Pack(cs2Val, ftIncr500,       2);
        char cs2Char = MapToAlphaNum(cs2Val);
        
        callsign[1] = cs2Char;
        
        // Callsign 3 is the second part of the ID
        callsign[2] = id_[1];
        
        // Callsign 4 is a straight copy of the 5th grid char
        callsign[3] = grid_[4];
        
        // Callsign 5 is a straight copy of the 6th grid char
        callsign[4] = grid_[5];
        
        // Callsign 6 is temperature and voltage
        uint8_t cs6Val = 0;
        cs6Val = Pack(cs6Val, temperatureCValue, 8);
        cs6Val = Pack(cs6Val, milliVoltValue,    3);
        char cs6Char = MapToAlphaSpace(cs6Val);
        
        callsign[5] = cs6Char;
        
        
        // Now use the WSPRMessage public interface to set the underlying values
        // as external viewers of a WSPRMessage would know them.
        WSPRMessage::SetCallsign(callsign);
        WSPRMessage::SetGrid(grid_);
        WSPRMessage::SetPower(power);
    }
    
    
private:

    static const uint8_t ID_LEN = 2;
    char id_[ID_LEN + 1] = { 0 };
    
    static const uint8_t GRID_LEN = 6;
    char grid_[GRID_LEN + 1] = { 0 };
    
    uint32_t altitudeFt_   = 0;
    uint8_t  speedKnots_   = 0;
    int8_t   temperatureC_ = 0;
    uint16_t milliVolt_    = 0;
};


#endif  // __WSPR_MESSAGE_PICO_TRACKER_WSPR_2_H__


































