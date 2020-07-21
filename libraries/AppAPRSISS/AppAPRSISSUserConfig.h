#ifndef __APP_APRS_ISS_USER_CONFIG_H__
#define __APP_APRS_ISS_USER_CONFIG_H__

struct AppAPRSISSUserConfig
{
    static const uint8_t CALLSIGN_LEN              = 6;
    static const uint8_t SYMBOL_TABLE_AND_CODE_LEN = 2;
    static const uint8_t COMMENT_LEN               = 16;
    
    // APRS Stuff
    // Symbol table: https://www.yachttrack.org/info_camper/downloads/APRS_Symbol_Chart.pdf
    struct
    {
        char    callsign[CALLSIGN_LEN + 1] = "KD2KDD";
        uint8_t ssid = 3;
        char    symbolTableAndCode[SYMBOL_TABLE_AND_CODE_LEN + 1] = "\\O";   // rocket
        char    comment[COMMENT_LEN + 1] = "Hi Mom via ISS!!";
    } aprs;

    struct
    {
        uint8_t  transmitCount           = 1;
        uint32_t delayMsBetweenTransmits = 3000;
    } radio;
};


#endif  // __APP_APRS_ISS_USER_CONFIG_H__





































