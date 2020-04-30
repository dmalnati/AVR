#ifndef __APP_PICO_TRACKER_WSPR_2_TEST_H__
#define __APP_PICO_TRACKER_WSPR_2_TEST_H__


#include "PAL.h"
#include "Evm.h"
#include "Log.h"
#include "SerialInput.h"
#include "AppPicoTrackerWSPR2TestableBase.h"


class AppPicoTrackerWSPR2Test
: AppPicoTrackerWSPR2TestableBase
{
public:

    AppPicoTrackerWSPR2Test(const AppPicoTrackerWSPR2Config &cfg)
    : AppPicoTrackerWSPR2TestableBase(cfg)
    , mtc_(userConfig_.radio.mtCalibration)
    , wsprChannel_(WSPRMessageTransmitter::WSPR_CHANNEL_DEFAULT)
    , freqInHundredths_(wsprMessageTransmitter_.GetCalculatedFreqHundredths())
    , onOff_(0)
    {
        // Nothing to do
    }

    void Run()
    {
        AppPicoTrackerWSPR2TestableBase::Init();
        
        console_.SetVerbose(0);
        
        SetupCommands();
        SetupInitialState();
        ExposeBitTransitions();
        
        
        console_.Start();
        PrintMenu();
        
        TerminalControl::ChangeColor(colorInput_);

        evm_.MainLoop();
    }
    
    
private:

    void TestRadioOn(uint8_t verbose = 1)
    {
        if (verbose)
        {
            Log(P("Radio On"));
        }

        // Combine the real application's two-step operation to a single
        // operation here.
        StartSubsystemWSPR();
        PreSendMessage();
        
        // Override real application's use of a random number for the
        // WSPR channel.
        wsprMessageTransmitter_.SetChannel(wsprChannel_);
        
        // Override real application's PreSendMessage functionality, so
        // instead of calling watchdog at periods, we see output on
        // the interface pin.
        ExposeBitTransitions();
        
        // Force the re-application of overridden configuration
        wsprMessageTransmitter_.RadioOn();
        
        // Keep our own state as to whether radio on or off
        onOff_ = 1;
    }

    void SetupCommands()
    {
        console_.RegisterErrorHandler([this](char *cmdStr){
            TerminalControl::ChangeColor(colorOutput_);
            
            Str str(cmdStr);
            
            uint8_t tokenCount = str.TokenCount(' ');
            
            const uint8_t BUF_SIZE = 30;
            char p1[BUF_SIZE+1] = { 0 };
            char p2[BUF_SIZE+1] = { 0 };
            char p3[BUF_SIZE+1] = { 0 };
            
            strncpy(p1, str.TokenAtIdx(0, ' '), BUF_SIZE);
            strncpy(p2, str.TokenAtIdx(1, ' '), BUF_SIZE);
            strncpy(p3, str.TokenAtIdx(2, ' '), BUF_SIZE);
            
            //////////////////////////////////////////////////////////////
            //
            // Basic Testing Commands
            //
            //////////////////////////////////////////////////////////////
            
            uint8_t printCurrentValues = 0;

            if (!strcmp_P(p1, P("test")))
            {
                if (!strcmp_P(p2, P("leds")))
                {
                    Log(P("Red"));
                    PAL.DigitalWrite(cfg_.pinLedRed, HIGH);
                    PAL.Delay(500);
                    PAL.DigitalWrite(cfg_.pinLedRed, LOW);
                    
                    Log(P("Green"));
                    PAL.DigitalWrite(cfg_.pinLedGreen, HIGH);
                    PAL.Delay(500);
                    PAL.DigitalWrite(cfg_.pinLedGreen, LOW);
                }
                else if (!strcmp_P(p2, P("tempC")))
                {
                    StartSubsystemTemperature();
                    int8_t tempC = sensorTemp_.GetTempC();
                    StopSubsystemTemperature();
                    
                    Log(P("tempC: "), tempC, ", tempF: ", ((tempC * (9.0 / 5.0)) + 32));
                }
                else if (!strcmp_P(p2, P("gps")))
                {
                    if (!strcmp_P(p3, P("on")))
                    {
                        Log(P("GPS ON"));
                        
                        StartSubsystemGPS();
                    }
                    else if (!strcmp_P(p3, P("off")))
                    {
                        Log(P("GPS OFF"));
                        
                        StopSubsystemGPS();
                    }
                }
            }
        

            //////////////////////////////////////////////////////////////
            //
            // Time Testing/Calibration Commands
            //
            //////////////////////////////////////////////////////////////
            
            if (!strcmp_P(p1, P("test")))
            {
                if (!strcmp_P(p2, P("systemClockOffsetMs")))
                {
                    PAL.PinMode(cfg_.pinConfigure, OUTPUT);

                    Log(P("Current systemClockOffsetMs = "), mtc_.systemClockOffsetMs);
                    Log(P("Measure with scope, get pulse widths to "), WSPRMessageTransmitter::WSPR_DELAY_MS, P(" ms"));
                    Log(P("Positive adjustments make pulse durations longer"));
                    Log(P("Negative adjustments make pulse durations shorter"));

                    for (uint8_t i = 0; i < 3; ++i)
                    {
                        PAL.DigitalWrite(cfg_.pinConfigure, HIGH);
                        PAL.Delay(WSPRMessageTransmitter::WSPR_DELAY_MS + mtc_.systemClockOffsetMs);
                        PAL.DigitalWrite(cfg_.pinConfigure, LOW);
                        PAL.Delay(WSPRMessageTransmitter::WSPR_DELAY_MS + mtc_.systemClockOffsetMs);
                    }

                    Log(P("Done"));
                }            
            }

            if (!strcmp_P(p1, P("set")))
            {
                if (!strcmp_P(p2, P("systemClockOffsetMs")) && tokenCount == 3)
                {
                    mtc_.systemClockOffsetMs = atol(p3);
                    
                    Log(P("Setting systemClockOffsetMs to "), mtc_.systemClockOffsetMs);

                    printCurrentValues = 1;

                    if (onOff_)
                    {
                        TestRadioOn();
                    }
                }
            }
            
            //////////////////////////////////////////////////////////////
            //
            // Frequency Testing/Calibration Commands
            //
            //////////////////////////////////////////////////////////////
            
            if (!strcmp_P(p1, P("test")))
            {
                if (!strcmp_P(p2, P("radio")))
                {
                    if (!strcmp_P(p3, P("on")))
                    {
                        TestRadioOn();
                    }
                    else if (!strcmp_P(p3, P("off")))
                    {
                        Log(P("Radio Off"));
                        
                        // Invoke the application's PostSendMessage
                        PostSendMessage();

                        onOff_ = 0;
                    }
                }
            }
            
            if (!strcmp_P(p1, P("set")))
            {
                if (!strcmp_P(p2, P("crystalCorrectionFactor")) && tokenCount == 3)
                {
                    mtc_.crystalCorrectionFactor = atol(p3);
                    
                    Log(P("Setting crystalCorrectionFactor to "), mtc_.crystalCorrectionFactor);

                    printCurrentValues = 1;

                    if (onOff_)
                    {
                        TestRadioOn(0);
                    }
                }
                else if (!strcmp_P(p2, P("wsprChannel")) && tokenCount == 3)
                {
                    uint8_t channel = atol(p3);
                    
                    wsprChannel_ = channel;

                    if (onOff_)
                    {
                        TestRadioOn(0);
                    }
                    
                    freqInHundredths_ = wsprMessageTransmitter_.GetCalculatedFreqHundredths();

                    Log(P("Setting wsprChannel to "), wsprChannel_, P(", freq now "), freqInHundredths_ / 100);

                    printCurrentValues = 1;
                }
                else if (!strcmp_P(p2, P("freq")) && tokenCount == 3)
                {
                    uint32_t val = atol(p3);

                    freqInHundredths_ = val * 100;
                    
                    Log(P("Setting freq to \""), freqInHundredths_ / 100, '"');

                    if (onOff_)
                    {
                        wsprMessageTransmitter_.SetFreqHundredths(freqInHundredths_);
                    }

                    printCurrentValues = 1;
                }
            }

            
            //////////////////////////////////////////////////////////////
            //
            // WPSR Send Commands
            //
            //////////////////////////////////////////////////////////////
            
            if (!strcmp_P(p1, P("set")))
            {
                if (!strcmp_P(p2, P("wsprCallsign")) && tokenCount == 3)
                {
                    const char *p = p3;
                    
                    Log(P("Setting WsprCallsign to \""), p, '"');

                    wsprMessageLiteral_.SetCallsign((char *)p);

                    printCurrentValues = 1;
                }
                else if (!strcmp_P(p2, P("grid")) && tokenCount == 3)
                {
                    Log(P("Setting Grid to \""), p3, '"');

                    wsprMessageLiteral_.SetGrid(p3);
                    wsprMessageEncoded_.SetGrid(p3);

                    printCurrentValues = 1;
                }
                else if (!strcmp_P(p2, P("powerDbm")) && tokenCount == 3)
                {
                    Log(P("Setting Power to \""), p3, '"');

                    wsprMessageLiteral_.SetPower(atoi(p3));

                    printCurrentValues = 1;
                }
                else if (!strcmp_P(p2, P("wsprCallsignId")) && tokenCount == 3)
                {
                    const char *p = p3;
                    
                    Log(P("Setting WsprCallsignId to \""), p, '"');

                    wsprMessageEncoded_.SetId((char *)p);

                    printCurrentValues = 1;
                }
                else if (!strcmp_P(p2, P("altitudeFt")) && tokenCount == 3)
                {
                    uint32_t val = atol(p3);
                    
                    Log(P("Setting AltitudeFt to \""), val, '"');

                    wsprMessageEncoded_.SetAltitudeFt(val);

                    printCurrentValues = 1;
                }
                else if (!strcmp_P(p2, P("speedKnots")) && tokenCount == 3)
                {
                    uint8_t val = atoi(p3);
                    
                    Log(P("Setting SpeedKnots to \""), val, '"');

                    wsprMessageEncoded_.SetSpeedKnots(val);

                    printCurrentValues = 1;
                }
                else if (!strcmp_P(p2, P("tempC")) && tokenCount == 3)
                {
                    int8_t val = atoi(p3);
                    
                    Log(P("Setting TempC to \""), val, '"');

                    wsprMessageEncoded_.SetTemperatureC(val);

                    printCurrentValues = 1;
                }
            }

            if (!strcmp_P(p1, P("test")))
            {
                if (!strcmp_P(p2, P("send")))
                {
                    Log(P("Sending both messages"));

                    if (!onOff_)
                    {
                        TestRadioOn();
                    }

                    printCurrentValues = 1;
                    
                    // Invoke the application's SendMessage function
                    SendMessages();

                    Log(P("Send complete"));
                }
                if (!strcmp_P(p2, P("sendL")))
                {
                    Log(P("Sending Literal"));

                    if (!onOff_)
                    {
                        TestRadioOn();
                    }

                    printCurrentValues = 1;
                    
                    // Invoke the application's SendMessage function
                    SendMessageLiteral();

                    Log(P("Send complete"));
                }
                if (!strcmp_P(p2, P("sendE")))
                {
                    Log(P("Sending Encoded"));

                    if (!onOff_)
                    {
                        TestRadioOn();
                    }

                    printCurrentValues = 1;
                    
                    // Invoke the application's SendMessage function
                    SendMessageEncoded();

                    Log(P("Send complete"));
                }
            }
            
            //////////////////////////////////////////////////////////////
            //
            // Misc
            //
            //////////////////////////////////////////////////////////////
            
            if (!strcmp_P(p1, P("help")))
            {
                PrintMenu();
            }
            
            if (printCurrentValues)
            {
                LogNL();
                PrintCurrentValues();
            }
            
            LogNL(2);
            
            TerminalControl::ChangeColor(colorInput_);
        });
    }
    
    void SetupInitialState()
    {
        // Set some defaults
        FillOutWSPRMessageLiteral();
        FillOutWSPRMessageEncoded();
        
        wsprMessageLiteral_.SetGrid("FN20XR");
        wsprMessageLiteral_.SetPower(10);

        // Override GPS-related due to not having a lock yet
        wsprMessageEncoded_.SetGrid("FN20XR");
        wsprMessageEncoded_.SetAltitudeFt(13500);
        wsprMessageEncoded_.SetSpeedKnots(76);
        wsprMessageEncoded_.SetTemperatureC(-30);
    }
    
    void ExposeBitTransitions()
    {
        // Toggle interface pin whenever a bit changes
        wsprMessageTransmitter_.SetCallbackOnBitChange([this](){
            PAL.PinMode(cfg_.pinConfigure, OUTPUT);
            PAL.DigitalToggle(cfg_.pinConfigure);
        });
    }
    
    
    
    
    
    
    void PrintHeading(PStr pStr, TerminalControl::Color c)
    {
        uint8_t len = strlen_P(pStr);
        
        TerminalControl::ChangeColor(c);
        
        Log(pStr);
        LogX('-', len);
    }
    
    void PrintMenu()
    {
        LogNL(4);

        PrintHeading(P("Basic Testing Commands"), colorHeaderCommands_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("test leds"));
        Log(P("test tempC"));
        Log(P("test gps on"));
        Log(P("test gps off"));
        LogNL();
        
        PrintHeading(P("Time Testing/Calibration Commands"), colorHeaderCommands_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("test systemClockOffsetMs"));
        Log(P("set  systemClockOffsetMs"));
        LogNL();
        
        PrintHeading(P("Frequency Testing/Calibration Commands"), colorHeaderCommands_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("test radio on"));
        Log(P("set  wsprChannel x"));
        Log(P("set  crystalCorrectionFactor"));
        Log(P("test radio off"));
        LogNL();
        
        PrintHeading(P("WPSR Send Commands"), colorHeaderCommands_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("set  wsprCallsign"));
        Log(P("set  grid"));
        Log(P("set  powerDbm"));
        LogNL();
        Log(P("set  wsprCallsignId"));
        Log(P("set  grid"));
        Log(P("set  altitudeFt"));
        Log(P("set  speedKnots"));
        Log(P("set  temperatureC"));
        LogNL();
        Log(P("test sendL"));
        Log(P("test sendE"));
        Log(P("test send"));
        LogNL();

        PrintHeading(P("Misc Commands"), colorHeaderCommands_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("help  - this menu"));
        Log(P("reset - reboot and start over"));
        LogNL();
        
        LogNL();
        PrintCurrentValues();
        LogNL();
    }
    
    
    void PrintCurrentValues()
    {
        const char *callsign = NULL;
        const char *grid     = NULL;
        uint8_t     powerDbm = 0;
        
        wsprMessageLiteral_.GetData(callsign, grid, powerDbm);
        PrintHeading(P("WSPR Literal Transmit Values"), colorHeaderValues_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("wsprCallsign: "), callsign);
        Log(P("grid        : "), grid);
        Log(P("powerDbm    : "), powerDbm);
        LogNL();
        
        PrintHeading(P("WSPR Encoded Values"), colorHeaderValues_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("wsprCallsignId: "), wsprMessageEncoded_.GetId());
        Log(P("grid          : "), wsprMessageEncoded_.GetGrid());
        Log(P("altitudeFt    : "), wsprMessageEncoded_.GetAltitudeFt());
        Log(P("speedKnots    : "), wsprMessageEncoded_.GetSpeedKnots());
        Log(P("tempC         : "), wsprMessageEncoded_.GetTemperatureC());
        LogNL();

        wsprMessageEncoded_.GetData(callsign, grid, powerDbm);
        PrintHeading(P("WSPR Encoded Transmit Values"), colorHeaderValues_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("wsprCallsign: "), callsign);
        Log(P("grid        : "), grid);
        Log(P("powerDbm    : "), powerDbm);
        LogNL();
        
        PrintHeading(P("WSPR Frequency Values"), colorHeaderValues_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("freq       : "), freqInHundredths_ / 100);
        Log(P("wsprChannel: "), wsprMessageTransmitter_.GetChannel());
        LogNL();
        
        PrintHeading(P("System Tunable Values"), colorHeaderValues_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("crystalCorrectionFactor: "), mtc_.crystalCorrectionFactor);
        Log(P("systemClockOffsetMs    : "), mtc_.systemClockOffsetMs);
    }




private:

    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    SerialAsyncConsoleEnhanced<0, 50>  console_;
    
    WSPRMessageTransmitter::Calibration &mtc_;
    
    uint8_t wsprChannel_;
    
    uint32_t freqInHundredths_;

    uint8_t onOff_;
    
    
    const TerminalControl::Color colorHeaderCommands_ = TerminalControl::Color::GREEN;
    const TerminalControl::Color colorHeaderValues_   = TerminalControl::Color::CYAN;
    const TerminalControl::Color colorItems_          = TerminalControl::Color::MAGENTA;
    const TerminalControl::Color colorInput_          = TerminalControl::Color::WHITE;

};








#endif  // __APP_PICO_TRACKER_WSPR_2_TEST_H__





































