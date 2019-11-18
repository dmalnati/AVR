#ifndef __APP_APRS_ISS_TESTABLE_BASE_H__
#define __APP_APRS_ISS_TESTABLE_BASE_H__


#include "PAL.h"
#include "Utl.h"
#include "Evm.h"
#include "Log.h"
#include "PinInput.h"
#include "TerminalControl.h"
#include "SensorGPSUblox.h"
#include "RFSI4463PRO.h"
#include "APRSPositionReportMessageAPRSISS.h"
#include "AX25UIMessageTransmitter.h"
#include "AppAPRSISSUserConfig.h"


struct AppAPRSISSConfig
{
    // Human interfacing
    uint8_t pinConfigure;
    uint8_t pinSend;
    uint8_t pinFreqSelect;

    // GPS
    uint8_t pinGpsSerialRx; // receive GPS data from this pin
    uint8_t pinGpsSerialTx; // send data to the GPS on this pin

    // Status LEDs
    uint8_t pinLedRed;
    uint8_t pinLedGreen;
    
    // Radio
    uint8_t pinRadioSlaveSelect;
    uint8_t pinRadioShutdown;
};


class AppAPRSISSTestableBase
{
public:

    AppAPRSISSTestableBase(const AppAPRSISSConfig &cfg)
    : cfg_(cfg)
    , pinInputSend_(cfg_.pinSend)
    , gps_(cfg_.pinGpsSerialRx, cfg_.pinGpsSerialTx)
    , radio_(cfg_.pinRadioSlaveSelect, cfg_.pinRadioShutdown)
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
        
        // Shut down subsystems
        // Floating pins have been seen to be enough to be high enough to
        // cause unintentional operation
        
        // GPS Subsystem
        StopSubsystemGPS();

        // Set up LED pins as output
        PAL.PinMode(cfg_.pinLedRed,   OUTPUT);
        PAL.PinMode(cfg_.pinLedGreen, OUTPUT);

        // Set up send button pin as input (pullup)
        PAL.PinMode(cfg_.pinSend, INPUT_PULLUP);

        // Set freq select button as input (pullup)
        PAL.PinMode(cfg_.pinFreqSelect, INPUT_PULLUP);

        // Init radio
        radio_.Init();
        
        // Set up APRS Message Sender
        amt_.Init([this](){ 
            PAL.WatchdogReset();
            radio_.Start();
        }, [this](){
            radio_.Stop();
            PAL.WatchdogReset();
        });
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // APRS Message Sending
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void SendMessage(SensorGPSUblox::Measurement &gpsMeasurement)
    {
        // Kick the watchdog
        PAL.WatchdogReset();

        AX25UIMessage &msg = *amt_.GetAX25UIMessage();

        msg.SetDstAddress("APZ001", 0);
        msg.SetSrcAddress(userConfig_.aprs.callsign, userConfig_.aprs.ssid);
        msg.AddRepeaterAddress("ARISS", 0);
        msg.AddRepeaterAddress("WIDE2", 1);

        // Add APRS data
        uint8_t *bufInfo    = NULL;
        uint8_t  bufInfoLen = 0;
        
        // Get buffer from AX25UIMessage to fill in APRS content
        if (msg.GetUnsafePtrInfo(&bufInfo, &bufInfoLen))
        {
            // Give buffer to helper interface
            APRSPositionReportMessageAPRSISS aprm;
            aprm.SetTargetBuf(bufInfo, bufInfoLen);
            
            // Fill out standard APRS fields
            aprm.SetTimeLocal(gpsMeasurement.hour,
                              gpsMeasurement.minute,
                              gpsMeasurement.second);
            aprm.SetLatitude(gpsMeasurement.latitudeDegrees,
                             gpsMeasurement.latitudeMinutes,
                             gpsMeasurement.latitudeSeconds);
            aprm.SetSymbolTableID(userConfig_.aprs.symbolTableAndCode[0]);
            aprm.SetLongitude(gpsMeasurement.longitudeDegrees,
                              gpsMeasurement.longitudeMinutes,
                              gpsMeasurement.longitudeSeconds);
            aprm.SetSymbolCode(userConfig_.aprs.symbolTableAndCode[1]);
            
            // Fill out extended standard APRS fields
            aprm.SetCommentCourseAndSpeed(gpsMeasurement.courseDegrees,
                                          gpsMeasurement.speedKnots);
            aprm.SetCommentAltitude(gpsMeasurement.altitudeFt);

            // Fill out my custom extensions
            aprm.AppendCommentString(" ");
            aprm.SetComment(userConfig_.aprs.comment);

            // Update message structure to know how many bytes we used
            msg.AssertInfoBytesUsed(aprm.GetBytesUsed());
        }

        // Configure and Transmit
        amt_.SetFlagStartDurationMs(300);
        amt_.SetFlagEndDurationMs(10);
        amt_.SetTransmitCount(userConfig_.radio.transmitCount);
        amt_.SetDelayMsBetweenTransmits(userConfig_.radio.delayMsBetweenTransmits);

        // Kick the watchdog before actual transmission
        PAL.WatchdogReset();

        SetFreqBasedOnSwitch();

        amt_.Transmit();
    }

    void SetFreqBasedOnSwitch()
    {
        // Set up radio frequency based on user config
        if (PAL.DigitalRead(cfg_.pinFreqSelect))
        {
            // decodable regular APRS freq
            const uint32_t APRS_TYP_FREQUENCY = 144390000UL;
            radio_.SetFrequency(APRS_TYP_FREQUENCY);

            Log(P("Using US land APRS freq of 144.390MHz"));
        }
        else
        {
            // Typical USA APRS freq
            const uint32_t APRS_ISS_FREQUENCY = 145825000UL;
            radio_.SetFrequency(APRS_ISS_FREQUENCY);

            Log(P("Using ISS APRS freq of 145.825MHz"));
        }
    }
    
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // GPS Controls
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void StartSubsystemGPS()
    {
        gps_.EnableSerialInput();
        gps_.EnableSerialOutput();
    }
    
    void ResetAndCutBatteryBackupSubsystemGPS()
    {
        gps_.ResetModule();
    }
    
    void StopSubsystemGPS()
    {
        // Work around a hardware issue where the GPS will draw lots of current
        // through a logic input
        gps_.DisableSerialOutput();
        
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
        // Intentionally not resetting the fix here.
        // The GPS will be stopped while transmitting, and prior locks
        // are ok to use in the future.
        //gps_.ResetFix();
        
        // cause the gps module to store the metadata is has learned from
        // the satellites it can see and used to get a lock.
        // this will be read again automatically by the module on startup.
        gps_.SaveConfiguration();

        // Disable subsystem
        StopSubsystemGPS();
    }
    
    
    
    
public:
    
    const AppAPRSISSConfig &cfg_;

    AppAPRSISSUserConfig userConfig_;

    PinInputNoIvm pinInputSend_;
    
    SensorGPSUblox  gps_;

    RFSI4463PRO                 radio_;
    AX25UIMessageTransmitter<>  amt_;

    const TerminalControl::Color colorOutput_ = TerminalControl::Color::YELLOW;

};





#endif  // __APP_APRS_ISS_TESTABLE_BASE_H__












