#ifndef __APP_HAB_TRACKER_1_H__
#define __APP_HAB_TRACKER_1_H__


#include "Evm.h"
#include "PeripheralOpenLog.h"
#include "SensorGPSUblox.h"
#include "SensorEcompassLSM303C.h"
#include "SensorBarometerBMP180.h"


struct AppHABTracker1Config
{
    // SD Logger
    uint8_t pinSerialTxSdLogger;
    
    // GPS
    uint8_t pinSerialRxGPS;
    uint8_t pinSerialTxGPS;
};


class AppHABTracker1
{
    static const uint8_t SD_LOGGER_FILE_HANDLE_COUNT = 1;
    
    using PeripheralOpenLogType = PeripheralOpenLog<SD_LOGGER_FILE_HANDLE_COUNT>;
    
public:
    AppHABTracker1(AppHABTracker1Config &cfg)
    : cfg_(cfg)
    , sdLogger_(cfg_.pinSerialTxSdLogger)
    , gps_(cfg_.pinSerialRxGPS, cfg_.pinSerialTxGPS)
    {
        // Nothing to do
    }
    
    ~AppHABTracker1() {}
    
    void Run()
    {
        // SD Logger
        sdLogger_.Init();
        
        // GPS
        gps_.Init();
        
        // Compass
        compass_.Init();
        
        // Barometer
        barometer_.Init();
        
        // Handle events
        evm_.MainLoop();
    }

private:

    void Notes()
    {
        // The barometer can give sea-level-compensated pressure readings, which
        // are standard, by additional processing using the altitude the
        // real pressure reading was taken at.
        
        // The GPS altitude should be used as that parameter.
    }

    
    
private:
    
    // Calculate sizing values for Evm
    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    
private:
    
    AppHABTracker1Config &cfg_;
    
    PeripheralOpenLogType   sdLogger_;
    SensorGPSUblox          gps_;
    SensorEcompassLSM303C   compass_;
    SensorBarometerBMP180   barometer_;
    
};


#endif  // __APP_HAB_TRACKER_1_H__



