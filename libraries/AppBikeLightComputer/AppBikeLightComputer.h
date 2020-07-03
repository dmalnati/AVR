#ifndef __APP_BIKE_LIGHT_COMPUTER_H__
#define __APP_BIKE_LIGHT_COMPUTER_H__


#include "Log.h"
#include "Evm.h"
#include "RgbLedEffectsController.h"


struct AppBikeLightComputerConfig
{
    uint32_t periodRedMs;
    uint32_t periodGreenMs;
    uint32_t periodBlueMs;
};


class AppBikeLightComputer
{
public:
    AppBikeLightComputer(AppBikeLightComputerConfig &cfg)
    : cfg_(cfg)
    {
        // Nothing to do
    }

    void Run()
    {
        LogStart(9600);
        Log(P("Starting"));

        rgb_.SetPeriodRed(cfg_.periodRedMs);
        rgb_.SetPeriodGreen(cfg_.periodGreenMs);
        rgb_.SetPeriodBlue(cfg_.periodBlueMs);

        rgb_.Start();
        
        evm_.MainLoop();
    }


private:

    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppBikeLightComputerConfig &cfg_;

    RgbLedEffectsController rgb_;
};


#endif  // __APP_BIKE_LIGHT_COMPUTER_H__