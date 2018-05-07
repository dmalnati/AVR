#ifndef __WSPR_MESSAGE_H__
#define __WSPR_MESSAGE_H__


#include <string.h>


class WSPRMessage
{
    static const uint8_t CALLSIGN_LEN = 6;
    static const uint8_t GRID_LEN     = 4;
    
public:

    uint8_t SetCallsign(const char *callsign)
    {
        uint8_t retVal = 0;
        
        strncpy(callsign_, callsign, CALLSIGN_LEN);
        
        return retVal;
    }
    
    uint8_t SetGrid(const char *grid)
    {
        uint8_t retVal = 0;
        
        strncpy(grid_, grid, GRID_LEN);
        
        return retVal;
    }
    
    uint8_t SetPower(uint8_t powerDbm)
    {
        uint8_t retVal = 0;
        
        powerDbm_ = powerDbm;
        
        return retVal;
    }
    
    void GetData(const char * &callsign, const char * &grid, uint8_t &powerDbm)
    {
        callsign = callsign_;
        grid     = grid_;
        powerDbm = powerDbm_;
    }

private:

    char     callsign_[CALLSIGN_LEN + 1] = { 0 };
    char     grid_[GRID_LEN + 1]         = { 0 };
    uint8_t  powerDbm_                   = 0;
};



#endif  // __WSPR_MESSAGE_H__





