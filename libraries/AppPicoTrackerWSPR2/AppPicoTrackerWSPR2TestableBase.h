#ifndef __APP_PICO_TRACKER_WSPR_2_TESTABLE_BASE_H__
#define __APP_PICO_TRACKER_WSPR_2_TESTABLE_BASE_H__


#include "PAL.h"
#include "Utl.h"
#include "Evm.h"
#include "Log.h"
#include "TerminalControl.h"
#include "SensorGPSUblox.h"
#include "SensorTemperatureMCP9808.h"
#include "WSPRMessageTransmitter.h"
#include "WSPRMessagePicoTrackerWSPR2.h"
#include "AppPicoTrackerWSPR2UserConfig.h"


struct AppPicoTrackerWSPR2Config
{
    // Human interfacing
    uint8_t pinConfigure;
    
    // GPS
    uint8_t pinGpsBackupPower;
    uint8_t pinGpsEnable;
    uint8_t pinGpsSerialRx; // receive GPS data from this pin
    uint8_t pinGpsSerialTx; // send data to the GPS on this pin
    
    uint32_t gpsMaxDurationLocationLockWaitMs;
    uint32_t gpsMaxDurationTimeLockWaitMs;
    uint32_t gpsDurationWaitPostResetMs;
    
    // WSPR TX
    uint8_t pinWsprTxEnable;
    
    // Status LEDs
    uint8_t pinLedRed;
    uint8_t pinLedGreen;
};


class AppPicoTrackerWSPR2TestableBase
{
public:

    AppPicoTrackerWSPR2TestableBase(const AppPicoTrackerWSPR2Config &cfg)
    : cfg_(cfg)
    , gps_(cfg_.pinGpsSerialRx, cfg_.pinGpsSerialTx)
    {
        // Nothing to do
    }
    
    void Init()
    {
        // Init serial and announce startup
        LogStart(9600);
        LogX('\n', 10);
        
        TerminalControl::ChangeColor(colorOutput_);
        
        Log(P("Starting"));
        
        if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_WATCHDOG)
        {
            Log(P("WDTR"));
        }
        else if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_BROWNOUT)
        {
            Log(P("BODR"));
        }

        // Mandate low BOD fuse
        uint16_t bod = PAL.GetFuseBODLimMilliVolts();
        if (bod != 1800)
        {
            Log(P("BOD WRONG ("), bod, P(") -- Set to 1800"));
            PAL.Delay(5000);
            PAL.SoftReset();
        }
        
        // Shut down subsystems
        // Floating pins have been seen to be enough to be high enough to
        // cause unintentional operation

        // Protect against hangs
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_8000_MS);
        
        // GPS Subsystem
        PAL.PinMode(cfg_.pinGpsBackupPower, OUTPUT);
        PAL.DigitalWrite(cfg_.pinGpsBackupPower, HIGH);
        PAL.PinMode(cfg_.pinGpsEnable, OUTPUT);
        StopSubsystemGPS();
        
        // WSPR Subsystem
        PAL.PinMode(cfg_.pinWsprTxEnable, OUTPUT);
        StopSubsystemWSPR();

        // Temperature Subsystem
        StartSubsystemTemperature();
        StopSubsystemTemperature();

        // Bring I2C bus offline since some components behave badly when not
        // powered but still connected to a live I2C bus
        StopI2C();

        // Disable watchdog as the main set of code which can hang is complete
        PAL.WatchdogDisable();
        
        // Set up LED control
        PAL.PinMode(cfg_.pinLedRed,   OUTPUT);
        PAL.PinMode(cfg_.pinLedGreen, OUTPUT);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // I2C Control
    //
    ///////////////////////////////////////////////////////////////////////////
    
    //
    // The I2C bus should be off unless there's a reason to have it on, and
    // that will be when a particular subsystem needs it.
    //
    // Expected use case:
    // - On system, subsystems will
    //   - maybe start
    //   - definitely stop
    // - Each subsystem will do this entirely before any other subsystem starts
    //
    // Instead of subsystems coordinating, this is a
    // reference-counted control over the I2C bus.

    void StartI2C()
    {
        if (i2cRefCount_ == 0)
        {
            // Transition from off to on
            I2C.BusOn();
        }

        ++i2cRefCount_;
    }

    void StopI2C()
    {
        if (i2cRefCount_ == 1)
        {
            // Transition from on to off
            I2C.BusOff();
        }

        if (i2cRefCount_ != 0)
        {
            --i2cRefCount_;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // WSPR Transmitter Control
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void StartSubsystemWSPR()
    {
        StartI2C();
        
        PAL.DigitalWrite(cfg_.pinWsprTxEnable, HIGH);
        
        const uint32_t WSPR_TCXO_STARTUP_TIME_MAX_MS = 10;
        PAL.Delay(WSPR_TCXO_STARTUP_TIME_MAX_MS);
    }
    
    void StopSubsystemWSPR()
    {
        PAL.DigitalWrite(cfg_.pinWsprTxEnable, LOW);
        
        StopI2C();
    }
    
    uint8_t PreSendMessage()
    {
        // Enable subsystem
        StartSubsystemWSPR();
        
        // Configure transmitter with calibration details
        wsprMessageTransmitter_.SetCalibration(userConfig_.radio.mtCalibration);
        
        // Set channel randomly
        uint8_t channel =
            Utl::GetRandomInRange(WSPRMessageTransmitter::WSPR_CHANNEL_LOW,
                                  WSPRMessageTransmitter::WSPR_CHANNEL_HIGH);
        wsprMessageTransmitter_.SetChannel(channel);
        
        // Set up the transmitter to kick the watchdog when sending data later
        wsprMessageTransmitter_.SetCallbackOnBitChange([](){
            PAL.WatchdogReset();
        });
        
        // Prepare system for send, warm up internals
        wsprMessageTransmitter_.RadioOn();
        
        return channel;
    }

    void SendMessageLiteral()
    {
        // Kick the watchdog
        PAL.WatchdogReset();
        
        // Send the message synchronously
        wsprMessageTransmitter_.Send(&wsprMessageLiteral_);
    }
    
    void SendMessageEncoded()
    {
        // Kick the watchdog
        PAL.WatchdogReset();
        
        // Send the message synchronously
        wsprMessageTransmitter_.Send(&wsprMessageEncoded_);
    }

    void SendMessages()
    {
        PAL.WatchdogReset();

        // First send the FCC-mandated message which reveals the
        // true callsign
        Log(P("Transmitting literal message"));
        SendMessageLiteral();

        // We want to now wait for the correct time to send the next
        // message.
        //
        // Duration of transmission of the first:
        // 162 symbols at 1.4648 baud = 110.6 s (110.592 = 1m50.592)
        //
        // We started the last at 1 second after a 2 min mark.
        // We want to do that again.  That's 2 min from the last start.
        // Since the prior transmission took 110.6 sec, we have
        // 120 - 110.6 = 9.4 seconds to wait.
        const uint32_t MS_WAIT = 9400;
        Log(P("Waiting for next 2-min mark"));
        PAL.Delay(MS_WAIT / 2);
        PAL.WatchdogReset();
        PAL.Delay(MS_WAIT / 2);
        PAL.WatchdogReset();

        // Now send the second encoded message
        Log(P("Transmitting encoded message"));
        SendMessageEncoded();
        
        PAL.WatchdogReset();
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
    
    void FillOutWSPRMessageLiteral()
    {
        // The boring regular one with real callsign which satisfies the FCC
        wsprMessageLiteral_.SetCallsign(userConfig_.wspr.callsign);
        wsprMessageLiteral_.SetGrid(gpsLocationMeasurement_.maidenheadGrid);
        wsprMessageLiteral_.SetPower(10);   // 10 mW == 10dBm
    }

    void FillOutWSPRMessageEncoded()
    {
        // Fill out actual message
        wsprMessageEncoded_.SetId(userConfig_.wspr.callsignId);
        wsprMessageEncoded_.SetGrid(gpsLocationMeasurement_.maidenheadGrid);
        wsprMessageEncoded_.SetAltitudeFt(gpsLocationMeasurement_.altitudeFt);
        wsprMessageEncoded_.SetSpeedKnots(gpsLocationMeasurement_.speedKnots);
        wsprMessageEncoded_.SetTemperatureC(tempC_);
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

    void StartSubsystemTemperature()
    {
        StartI2C();

        // Sadly the WSPR transmitter misbehaves and pulls down the I2C bus.
        // Just start it up before taking the temp, as the temp is the only
        // other I2C peripheral.  I don't want to add more parts to the system
        // if I can solve it in software.
        StartSubsystemWSPR();

        sensorTemp_.Wake();

        PAL.Delay(SensorTemperatureMCP9808::MS_MAX_TEMP_SENSE_DURATION);
    }

    void StopSubsystemTemperature()
    {
        sensorTemp_.Sleep();

        StopSubsystemWSPR();

        StopI2C();
    }

    
public:
    
    const AppPicoTrackerWSPR2Config &cfg_;

    AppPicoTrackerWSPR2UserConfig userConfig_;
    
    TimedEventHandlerDelegate tedWake_;

    uint8_t i2cRefCount_ = 0;
    
    SensorGPSUblox               gps_;
    SensorGPSUblox::Measurement  gpsLocationMeasurement_;
    SensorGPSUblox::Measurement  gpsTimeMeasurement_;

    SensorTemperatureMCP9808  sensorTemp_;
    int8_t                    tempC_ = 0;
    
    WSPRMessage                 wsprMessageLiteral_;
    WSPRMessagePicoTrackerWSPR2 wsprMessageEncoded_;
    WSPRMessageTransmitter      wsprMessageTransmitter_;
    
    const TerminalControl::Color colorOutput_ = TerminalControl::Color::YELLOW;

};





#endif  // __APP_PICO_TRACKER_WSPR_2_TESTABLE_BASE_H__












