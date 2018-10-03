#ifndef __APP_EVIL_EYE_H__
#define __APP_EVIL_EYE_H__


#include "Log.h"
#include "Evm.h"
#include "LedMatrixTimeMultiplexer.h"


struct AppEvilEyeConfig
{
};


class AppEvilEye
{
public:
    
    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
    
private:
    
    static const uint8_t ROW_COUNT = 3;
    static const uint8_t COL_COUNT = 2;
    

    
public:
    AppEvilEye(AppEvilEyeConfig &cfg)
    : cfg_(cfg)
    , mux_(
        { 19, 18, 17 },
        { 16, 15 }
      )
    {
        // Nothing to do
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Entry point
    //
    ///////////////////////////////////////////////////////////////////////////

    void Run()
    {
        // Init serial and announce startup
        LogStart(9600);
        Log(P("Starting"));
        
        
        
        
        
        
        
        // Start up LEDs
        mux_.Start();
        
        
        // Handle async events
        Log(P("Running"));
        PAL.Delay(1000);

        evm_.MainLoop();
    }
    
    
private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppEvilEyeConfig &cfg_;
    
    
    LedMatrixTimeMultiplexer<ROW_COUNT, COL_COUNT> mux_;

    
};

























#endif  // __APP_EVIL_EYE_H__