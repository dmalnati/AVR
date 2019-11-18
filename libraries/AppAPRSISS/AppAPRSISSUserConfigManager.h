#ifndef __APP_APRS_ISS_USER_CONFIG_MANAGER_H__
#define __APP_APRS_ISS_USER_CONFIG_MANAGER_H__


#include "SerialInput.h"
#include "StrFormat.h"
#include "AppAPRSISSUserConfig.h"


class AppAPRSISSUserConfigManager
: public PersistantConfigManager<AppAPRSISSUserConfig, 6, 0, 1>
{
public:

    AppAPRSISSUserConfigManager(uint8_t pinConfigure, AppAPRSISSUserConfig &config)
    : PersistantConfigManager(pinConfigure, config)
    {
        // Register formatter for uint32_t types which represent durations in ms
        idxFormatter_ = Menu().RegisterFormatter([](MenuType::Param &param){
            char buf[StrFormat::HHMMSSMMM_BUF_SIZE_NEEDED] = { 0 };
            
            StrFormat::DurationMsToHHMMSSMMM(*(uint32_t *)param.paramPtr, buf);
            StrFormat::TrimMsFromHHMMSSMMM(buf);
            StrFormat::TrimLeadingPaddingFromHHMMSSMMM(buf);
            
            LogNNL(buf);
        });
    }
    
private:

    void SetupMenu()
    {
        Menu().RegisterParamSTR(P("callsign"),
                                Config().aprs.callsign,
                                AppAPRSISSUserConfig::CALLSIGN_LEN);
        Menu().RegisterParamU8(P("ssid"),
                               &Config().aprs.ssid);
        Menu().RegisterParamSTR(P("symbolTableAndCode"),
                                Config().aprs.symbolTableAndCode,
                                AppAPRSISSUserConfig::SYMBOL_TABLE_AND_CODE_LEN);
        Menu().RegisterParamSTR(P("comment"),
                                Config().aprs.comment,
                                AppAPRSISSUserConfig::COMMENT_LEN);

        Menu().RegisterParamU8(P("transmitCount"),
                               &Config().radio.transmitCount);
        Menu().RegisterParamU32(P("delayMsBetweenTransmits"),
                                &Config().radio.delayMsBetweenTransmits, idxFormatter_);
    }

    int8_t idxFormatter_ = -1;
};


#endif  // __APP_APRS_ISS_USER_CONFIG_MANAGER_H__





















