#ifndef __APP_PICO_TRACKER_WSPR_1_TEST_H__
#define __APP_PICO_TRACKER_WSPR_1_TEST_H__


#include "PAL.h"
#include "Evm.h"
#include "Log.h"
#include "SerialInput.h"
#include "AppPicoTrackerWSPR1TestableBase.h"


class AppPicoTrackerWSPR1Test
: AppPicoTrackerWSPR1TestableBase
{
public:

    AppPicoTrackerWSPR1Test(const AppPicoTrackerWSPR1Config &cfg)
    : AppPicoTrackerWSPR1TestableBase(cfg)
    , mtc_(userConfig_.radio.mtCalibration)
    , freqInHundredths_(wsprMessageTransmitter_.GetCalculatedFreqHundredths())
    , onOff_(0)
    {
        // Nothing to do
    }

    void Run()
    {
        AppPicoTrackerWSPR1TestableBase::Init();
        
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

    void TestRadioOn()
    {
        Log(P("Radio On"));

        // Combine the real application's two-step operation to a single
        // operation here.
        StartSubsystemWSPR();
        PreSendMessage();
        
        // Override real application's PreSendMessage functionality, so
        // instead of calling watchdog at periods, we see output on
        // the interface pin.
        ExposeBitTransitions();
        
        // Add extra control over initial frequency
        wsprMessageTransmitter_.SetFreqHundredths(freqInHundredths_);

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
                    int8_t tempC = GetTemperatureC();
                    
                    Log(P("tempC: "), tempC, ", tempF: ", ((tempC * (9.0 / 5.0)) + 32));
                }
                else if (!strcmp_P(p2, P("milliVolts")))
                {
                    uint16_t inputMilliVolt = GetInputMilliVoltage();
                    
                    Log(P("milliVolts: "), inputMilliVolt);
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

                    Log(P("Testing system clock"));
                    Log(P("Current systemClockOffsetMs = "), mtc_.systemClockOffsetMs);
                    Log(P("Measure with scope, get pulse widths to "), WSPRMessageTransmitter::WSPR_DELAY_MS, P(" ms"));
                    Log(P("Positive adjustments make pulses smaller"));
                    Log(P("negative adjustments make pulses larger"));
                    Log(P("Starting test"));

                    for (uint8_t i = 0; i < 3; ++i)
                    {
                        PAL.DigitalWrite(cfg_.pinConfigure, HIGH);
                        PAL.Delay(WSPRMessageTransmitter::WSPR_DELAY_MS - mtc_.systemClockOffsetMs);
                        PAL.DigitalWrite(cfg_.pinConfigure, LOW);
                        PAL.Delay(WSPRMessageTransmitter::WSPR_DELAY_MS - mtc_.systemClockOffsetMs);
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
                else if (!strcmp_P(p2, P("chan")) && tokenCount == 3)
                {
                    uint32_t chan = atol(p3);

                    freqInHundredths_ =
                        (WSPRMessageTransmitter::WSPR_DEFAULT_DIAL_FREQ * 100UL) +
                        (WSPRMessageTransmitter::WSPR_OFFSET_FROM_DIAL_TO_USABLE_HZ * 100UL) +
                        (chan * WSPRMessageTransmitter::WSPR_CHANNEL_BANDWIDTH_HUNDREDTHS_HZ);

                    Log(P("Setting channel to "), chan, P(", freq now "), freqInHundredths_ / 100.0);

                    if (onOff_)
                    {
                        wsprMessageTransmitter_.SetFreqHundredths(freqInHundredths_);
                    }

                    printCurrentValues = 1;
                }
                else if (!strcmp_P(p2, P("freq")) && tokenCount == 3)
                {
                    uint32_t val = atol(p3);

                    freqInHundredths_ = val * 100;
                    
                    Log(P("Setting freq to \""), freqInHundredths_ / 100.0, '"');

                    if (onOff_)
                    {
                        wsprMessageTransmitter_.SetFreqHundredths(freqInHundredths_);
                    }

                    printCurrentValues = 1;
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
                        TestRadioOn();
                    }
                }
            }

            
            //////////////////////////////////////////////////////////////
            //
            // WPSR Send Commands
            //
            //////////////////////////////////////////////////////////////
            
            if (!strcmp_P(p1, P("set")))
            {
                if (!strcmp_P(p2, P("id")) && tokenCount == 3)
                {
                    const char *p = p3;
                    
                    Log(P("Setting ID to \""), p, '"');

                    wsprMessage_.SetId((char *)p);

                    printCurrentValues = 1;
                }
                else if (!strcmp_P(p2, P("altitudeFt")) && tokenCount == 3)
                {
                    uint32_t val = atol(p3);
                    
                    Log(P("Setting AltitudeFt to \""), val, '"');

                    wsprMessage_.SetAltitudeFt(val);

                    printCurrentValues = 1;
                }
                else if (!strcmp_P(p2, P("speedKnots")) && tokenCount == 3)
                {
                    uint8_t val = atoi(p3);
                    
                    Log(P("Setting SpeedKnots to \""), val, '"');

                    wsprMessage_.SetSpeedKnots(val);

                    printCurrentValues = 1;
                }
                else if (!strcmp_P(p2, P("tempC")) && tokenCount == 3)
                {
                    int8_t val = atoi(p3);
                    
                    Log(P("Setting TempC to \""), val, '"');

                    wsprMessage_.SetTemperatureC(val);

                    printCurrentValues = 1;
                }
                else if (!strcmp_P(p2, P("milliVolts")) && tokenCount == 3)
                {
                    uint16_t val = atol(p3);
                    
                    Log(P("Setting MilliVolts to \""), val, '"');

                    wsprMessage_.SetMilliVoltage(val);

                    printCurrentValues = 1;
                }
            }

            if (!strcmp_P(p1, P("test")))
            {
                if (!strcmp_P(p2, P("send")))
                {
                    Log(P("Sending"));

                    if (!onOff_)
                    {
                        TestRadioOn();
                    }

                    printCurrentValues = 1;
                    
                    // Invoke the application's SendMessage function
                    SendMessage();

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
        FillOutStandardWSPRMessage();
        
        // Override GPS-related due to not having a lock yet
        wsprMessage_.SetGrid("FN20XR");
        wsprMessage_.SetAltitudeFt(13500);
        wsprMessage_.SetSpeedKnots(76);
        
        // Override temp/voltage for now just for a standard startup value
        wsprMessage_.SetTemperatureC(-30);
        wsprMessage_.SetMilliVoltage(3100);
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
        Log(P("test milliVolts"));
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
        Log(P("test chan 0"));
        Log(P("test radio off"));
        Log(P("set  crystalCorrectionFactor"));
        LogNL();
        
        PrintHeading(P("WPSR Send Commands"), colorHeaderCommands_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("set  id"));
        Log(P("set  altitudeFt"));
        Log(P("set  speedKnots"));
        Log(P("set  temperatureC"));
        Log(P("set  milliVolts"));
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
        
        wsprMessage_.GetData(callsign, grid, powerDbm);
        
        PrintHeading(P("WSPR Encoded Values"), colorHeaderValues_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("id        : "), wsprMessage_.GetId());
        Log(P("grid      : "), wsprMessage_.GetGrid());
        Log(P("altitudeFt: "), wsprMessage_.GetAltitudeFt());
        Log(P("speedKnots: "), wsprMessage_.GetSpeedKnots());
        Log(P("tempC     : "), wsprMessage_.GetTemperatureC());
        Log(P("milliVolts: "), wsprMessage_.GetMilliVoltage());
        LogNL();
        
        PrintHeading(P("WSPR Transmit Values"), colorHeaderValues_);
        TerminalControl::ChangeColor(colorItems_);
        Log(P("freq    : "), freqInHundredths_ / 100.0);
        Log(P("callsign: "), callsign);
        Log(P("grid    : "), grid);
        Log(P("powerDbm: "), powerDbm);
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
    
    uint32_t freqInHundredths_;

    uint8_t onOff_;
    
    
    const TerminalControl::Color colorHeaderCommands_ = TerminalControl::Color::GREEN;
    const TerminalControl::Color colorHeaderValues_   = TerminalControl::Color::CYAN;
    const TerminalControl::Color colorItems_          = TerminalControl::Color::MAGENTA;
    const TerminalControl::Color colorInput_          = TerminalControl::Color::WHITE;

};








#endif  // __APP_PICO_TRACKER_WSPR_1_TEST_H__





































