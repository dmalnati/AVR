#ifndef __WSPR_MESSAGE_H__
#define __WSPR_MESSAGE_H__


#include <string.h>


class WSPRMessage
{
    static const uint8_t CALLSIGN_LEN = 6;
    static const uint8_t GRID_LEN     = 6;
    
public:

    void SetCallsign(const char *callsign)
    {
        strncpy(callsign_, callsign, CALLSIGN_LEN);
    }
    
    void SetGrid(const char *grid)
    {
        strncpy(grid_, grid, GRID_LEN);
    }
    
    void SetPower(uint8_t powerDbm)
    {
        powerDbm_ = powerDbm;
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





