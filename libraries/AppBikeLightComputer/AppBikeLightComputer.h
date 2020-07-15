#ifndef __APP_BIKE_LIGHT_COMPUTER_H__
#define __APP_BIKE_LIGHT_COMPUTER_H__


#include "Log.h"
#include "Evm.h"
#include "SerialInput.h"
#include "RFLink.h"
#include "RgbLedEffectsController.h"


struct AppBikeLightComputerConfig
{
    // interfacing config
    uint8_t pinConfigure;

    // radio config
    uint8_t pinIrq;
    uint8_t pinSdn;
    uint8_t pinSel;

    // initial settings
    uint32_t periodRedMs;
    uint32_t periodGreenMs;
    uint32_t periodBlueMs;
};


class AppBikeLightComputer
{
private:

    struct AppBikeLightComputerUserConfig
    {
        uint8_t addr  = 0;
        uint8_t realm = 0;
    };

    class AppBikeLightComputerConfigManager
    : public PersistantConfigManager<AppBikeLightComputerUserConfig, 2>
    {
    public:
        using PersistantConfigManager::PersistantConfigManager;
        
    private:
        virtual void SetupMenu() override
        {
            Menu().RegisterParamU8(P("addr"),   &Config().addr);
            Menu().RegisterParamU8(P("realm"),  &Config().realm);
        }
    };


public:
    AppBikeLightComputer(AppBikeLightComputerConfig &cfg)
    : cfg_(cfg)
    , radio_(cfg_.pinIrq, cfg_.pinSdn, cfg_.pinSel)
    {
        // Nothing to do
    }

    void Run()
    {
        LogStart(9600);
        Log(P("Starting"));

        // Get user config
        uint8_t userConfigOk = 0;
        
        {
            AppBikeLightComputerConfigManager mgr(cfg_.pinConfigure, userConfig_, 0);
            
            // For use in testing out different configurations
            uint8_t letDefaultApplyAutomatically = 1;
            userConfigOk = mgr.GetConfig(letDefaultApplyAutomatically);
        }

        if (userConfigOk)
        {
            Log(P("Config OK"));
            LogNL();

            SetUpCommandHandler();
            SetUpEffects();
            SetUpRadio();

            evm_.MainLoop();
        }
        else
        {
            Log(P("Bad Configuration"));
        }
    }


private:

    void SetUpCommandHandler()
    {
        console_.RegisterCommand("start", [this](char *){
            Log(P("Start"));
            rgb_.Start();
        });

        console_.RegisterCommand("pause", [this](char *){
            Log(P("Pause"));
            rgb_.Pause();
        });
        
        console_.RegisterCommand("stop", [this](char *){
            Log(P("Stop"));
            rgb_.Stop();
        });








        console_.RegisterCommand("status", [this](char *){
            radio_.DumpStatus();
        });

        console_.Start();
    }

    void SetUpEffects()
    {
        rgb_.SetPeriodRed(cfg_.periodRedMs);
        rgb_.SetPeriodGreen(cfg_.periodGreenMs);
        rgb_.SetPeriodBlue(cfg_.periodBlueMs);

        rgb_.Start();
    }

    void SetUpRadio()
    {
        radio_.Init();

        radio_.SetSrcAddr(userConfig_.addr);
        radio_.SetRealm(userConfig_.realm);
        radio_.SetReceiveBroadcast(1);

        radio_.SetOnMessageReceivedCallback([this](RFLinkHeader *, uint8_t *buf, uint8_t bufLen){
            if (buf[bufLen - 1] == '\0')
            {
                console_.Exec((const char *)buf);
            }
        });

        radio_.ModeReceive();
    }


private:

    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppBikeLightComputerConfig     &cfg_;
    AppBikeLightComputerUserConfig  userConfig_;

    SerialAsyncConsoleEnhanced<4>  console_;

    RFLink radio_;

    RgbLedEffectsController rgb_;
};


#endif  // __APP_BIKE_LIGHT_COMPUTER_H__