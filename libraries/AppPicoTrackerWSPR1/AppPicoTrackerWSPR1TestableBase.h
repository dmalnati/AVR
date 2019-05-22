#ifndef __APP_PICO_TRACKER_WSPR_1_TESTABLE_BASE_H__
#define __APP_PICO_TRACKER_WSPR_1_TESTABLE_BASE_H__


#include "PAL.h"
#include "Evm.h"
#include "Log.h"
#include "TerminalControl.h"
#include "SensorGPSUblox.h"
#include "WSPRMessageTransmitter.h"
#include "WSPRMessagePicoTrackerWSPR1.h"
#include "AppPicoTrackerWSPR1UserConfig.h"


struct AppPicoTrackerWSPR1Config
{
    // Human interfacing
    uint8_t pinConfigure;
    
    // Pre-regulator power sensing
    uint8_t pinInputVoltage;
    
    // Regulator control
    uint8_t pinRegPowerSaveEnable;
    
    // Solar configuration
    uint32_t intervalSolarWakeupMs;
    
    // GPS
    uint8_t pinGpsBackupPower;
    uint8_t pinGpsEnable;
    uint8_t pinGpsSerialRx; // receive GPS data from this pin
    uint8_t pinGpsSerialTx; // send data to the GPS on this pin
    
    uint32_t gpsMaxAgeLocationLockMs;
    uint32_t gpsMaxDurationTimeLockWaitMs;
    uint32_t gpsDurationWaitPostResetMs;
    
    // WSPR TX
    uint8_t pinWsprTxEnable;
    
    // Temperature sensor
    uint8_t pinTempSensorEnable;
    uint8_t pinTempSensorVoltageSense;

    // Status LEDs
    uint8_t pinLedRed;
    uint8_t pinLedGreen;
};


class AppPicoTrackerWSPR1TestableBase
{
public:

    AppPicoTrackerWSPR1TestableBase(const AppPicoTrackerWSPR1Config &cfg)
    : cfg_(cfg)
    , gps_(cfg_.pinGpsSerialRx, cfg_.pinGpsSerialTx)
    , gpsLocationLockOk_(0)
    {
        // Nothing to do
    }
    
    void Init()
    {
        uint8_t tcxoInUse = 0;
        
        // Handle external clock being configured.
        if (PAL.GetFuseExternalClockConfigured())
        {
            // If fuse indicates external clock, it's a 16 MHz TCXO.
            // We want to run at 8 MHz.  So prescale by 2.
            PAL.SetCpuPrescaler(PlatformAbstractionLayer::CpuPrescaler::DIV_BY_2);
            
            tcxoInUse = 1;
        }
        
        // Init serial and announce startup
        LogStart(9600);
        LogX('\n', 10);
        
        TerminalControl::ChangeColor(colorOutput_);
        
        LogNNL(P("Starting, TCXO "));
        Log(tcxoInUse ? P("Enabled") : P("Disabled"));
        
        
        if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_WATCHDOG)
        {
            Log(P("WDTR"));
        }
        else if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_BROWNOUT)
        {
            Log(P("BODR"));
        }
        
        // Don't allow being too low of a BOD.
        if (PAL.GetFuseBODLimMilliVolts() != 2700)
        {
            Log(P("BOD WRONG -- Set to 2700"));
            PAL.SoftReset();
        }
        
        // Set up control over regulator power save mode.
        PAL.PinMode(cfg_.pinRegPowerSaveEnable, OUTPUT);
        RegulatorPowerSaveDisable();
        
        // Shut down subsystems
        // Floating pins have been seen to be enough to be high enough to
        // cause unintentional operation
        
        // GPS Subsystem
        PAL.PinMode(cfg_.pinGpsBackupPower, OUTPUT);
        PAL.DigitalWrite(cfg_.pinGpsBackupPower, HIGH);
        PAL.PinMode(cfg_.pinGpsEnable, OUTPUT);
        StopSubsystemGPS();
        
        // WSPR Subsystem
        PAL.PinMode(cfg_.pinWsprTxEnable, OUTPUT);
        StopSubsystemWSPR();
        
        // Temperature
        PAL.PinMode(cfg_.pinTempSensorEnable, OUTPUT);
        GetTemperatureC();
        
        // Set up LEDs and blink to indicate power up
        PAL.PinMode(cfg_.pinLedRed,   OUTPUT);
        PAL.PinMode(cfg_.pinLedGreen, OUTPUT);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // WSPR Transmitter Control
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void StartSubsystemWSPR()
    {
        PAL.DigitalWrite(cfg_.pinWsprTxEnable, HIGH);
        
        const uint32_t WSPR_TCXO_STARTUP_TIME_MAX_MS = 10;
        PAL.Delay(WSPR_TCXO_STARTUP_TIME_MAX_MS);
    }
    
    void StopSubsystemWSPR()
    {
        PAL.DigitalWrite(cfg_.pinWsprTxEnable, LOW);
    }
    
    void PreSendMessage()
    {
        // Enable subsystem
        StartSubsystemWSPR();
        
        // Configure transmitter with calibration details
        wsprMessageTransmitter_.SetCalibration(userConfig_.radio.mtCalibration);
        
        // Set up the transmitter to kick the watchdog when sending data later
        wsprMessageTransmitter_.SetCallbackOnBitChange([](){
            PAL.WatchdogReset();
        });
        
        // Prepare system for send, warm up internals
        wsprMessageTransmitter_.RadioOn();
    }

    void SendMessage()
    {
        // Kick the watchdog
        PAL.WatchdogReset();
        
        // Send the message synchronously
        wsprMessageTransmitter_.Send(&wsprMessage_);
    }
    
    void PostSendMessage()
    {
        // Go back to idle state
        wsprMessageTransmitter_.RadioOff();
        
        // Disable subsystem
        StopSubsystemWSPR();
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // WSPR Message Construction
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void FillOutStandardWSPRMessage()
    {
        // Fill out actual message
        wsprMessage_.SetId(userConfig_.wspr.callsignId);
        wsprMessage_.SetGrid(gpsLocationMeasurement_.maidenheadGrid);
        wsprMessage_.SetAltitudeFt(gpsLocationMeasurement_.altitudeFt);
        wsprMessage_.SetSpeedKnots(gpsLocationMeasurement_.speedKnots);
        wsprMessage_.SetTemperatureC(GetTemperatureC());
        wsprMessage_.SetMilliVoltage(GetInputMilliVoltage());
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Power Regulator Controls
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void RegulatorPowerSaveEnable()
    {
        PAL.DigitalWrite(cfg_.pinRegPowerSaveEnable, LOW);
    }
    
    void RegulatorPowerSaveDisable()
    {
        PAL.DigitalWrite(cfg_.pinRegPowerSaveEnable, HIGH);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // GPS Controls
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void StartSubsystemGPS()
    {
        gps_.EnableSerialInput();
        
        PAL.DigitalWrite(cfg_.pinGpsBackupPower, HIGH);
        PAL.DigitalWrite(cfg_.pinGpsEnable, HIGH);
        
        gps_.EnableSerialOutput();
    }
    
    void ResetAndCutBatteryBackupSubsystemGPS()
    {
        gps_.ResetModule();
        
        // Cut the backup power, starving out any retained GPS state.
        //
        // The method to start the GPS subsystem start will re-enable
        // this backup power.  Notably the subsystem stop will not
        // disable it.  So, asymetric, on purpose.
        //
        // This leads to the backup power to the GPS being disabled for
        // the duration of time the tracker sleeps for before trying
        // again.
        PAL.DigitalWrite(cfg_.pinGpsBackupPower, LOW);
    }
    
    void StopSubsystemGPS()
    {
        // Work around a hardware issue where the GPS will draw lots of current
        // through a logic input
        gps_.DisableSerialOutput();
        
        // disable power supply to GPS
        // (battery backup for module-stored data supplied through other pin)
        PAL.DigitalWrite(cfg_.pinGpsEnable, LOW);
        
        // stop interrupts from firing in underlying code
        gps_.DisableSerialInput();
    }
    
    void StartGPS()
    {
        // Enable subsystem
        StartSubsystemGPS();
        
        // re-init to begin cycle again
        gps_.Init();
        
        // assert this is a high-altitude mode
        gps_.SetHighAltitudeMode();
    }

    void StopGPS()
    {
        // Don't allow prior fix to be used, we want a brand new fix to be
        // acquired next time the GPS starts up
        gps_.ResetFix();
        
        // cause the gps module to store the metadata is has learned from
        // the satellites it can see and used to get a lock.
        // this will be read again automatically by the module on startup.
        gps_.SaveConfiguration();

        // Disable subsystem
        StopSubsystemGPS();
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Temperature
    //
    ///////////////////////////////////////////////////////////////////////////
    
    int8_t GetTemperatureC()
    {
        // Put voltage across voltage divider
        PAL.DigitalWrite(cfg_.pinTempSensorEnable, HIGH);
        
        // Take 8 samples and average them to reduce transients
        uint16_t adcVal = PAL.AnalogRead1V1(cfg_.pinTempSensorVoltageSense, 1);
        
        // Cut power
        PAL.DigitalWrite(cfg_.pinTempSensorEnable, LOW);
        
        // Calculate actual millivolts sensed
        constexpr double MV_STEP = 1.07421875;
        uint16_t mvSensed = adcVal * MV_STEP;
        
        // Apply formula calculated in spreadsheet.
        // This is, at -50C, we expect to see ~42mV.
        // From there, we expect to see a 1mV change for every 5C temperature
        // change in the range of temperatures from -50 C to 20 C.
        int8_t tempC =  -50 + ((mvSensed - 42) * 5);
        
        return tempC;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Input Voltage
    //
    ///////////////////////////////////////////////////////////////////////////
    
    uint16_t GetInputMilliVoltage()
    {
        // Circuit has a voltage divider halve the input voltage.
        // Use the 8x sample option.
        uint16_t adcVal = PAL.AnalogRead(cfg_.pinInputVoltage, 1);
        
        // Scale to account for input divider, also scale to input millivolts
        constexpr double FACTOR = (2.0 * 3300.0 / 1024.0);
        uint16_t inputMilliVolt = adcVal * FACTOR;
        
        return inputMilliVolt;
    }
    
    
    
public:
    
    const AppPicoTrackerWSPR1Config &cfg_;

    AppPicoTrackerWSPR1UserConfig userConfig_;
    
    uint8_t inLowAltitudeStickyPeriod_ = 1;
    
    TimedEventHandlerDelegate tedWake_;
    
    SensorGPSUblox               gps_;
    SensorGPSUblox::Measurement  gpsLocationMeasurement_;
    uint8_t                      gpsLocationLockOk_;
    SensorGPSUblox::Measurement  gpsTimeMeasurement_;
    
    WSPRMessagePicoTrackerWSPR1 wsprMessage_;
    WSPRMessageTransmitter      wsprMessageTransmitter_;
    
    
    const TerminalControl::Color colorOutput_ = TerminalControl::Color::YELLOW;

};





#endif  // __APP_PICO_TRACKER_WSPR_1_TESTABLE_BASE_H__












