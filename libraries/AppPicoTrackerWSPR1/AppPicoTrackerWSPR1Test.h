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
        console_.Exec("help");
    
        Log("Running");
        evm_.MainLoop();
    }
    
    
private:

    void SetupCommands()
    {
        //////////////////////////////////////////////////////////////
        //
        // Traditional WSPR message settings
        //
        //////////////////////////////////////////////////////////////

        console_.RegisterCommand("callsign", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                const char *p = str.TokenAtIdx(1, ' ');
                
                Log(P("Setting callsign to \""), p, '"');

                wsprMessage_.SetCallsign((char *)p);

                PrintCurrentValues();
            }
        });

        console_.RegisterCommand("grid", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                const char *p = str.TokenAtIdx(1, ' ');
                
                Log(P("Setting grid to \""), p, '"');

                wsprMessage_.SetGrid((char *)p);

                PrintCurrentValues();
            }
        });
        
        console_.RegisterCommand("power", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                uint8_t val = atoi(str.TokenAtIdx(1, ' '));
                
                Log(P("Setting power to \""), val, '"');

                wsprMessage_.SetPower(val);

                PrintCurrentValues();
            }
        });
        
        //////////////////////////////////////////////////////////////
        //
        // Encoded WSPR message settings
        //
        //////////////////////////////////////////////////////////////

        console_.RegisterCommand("id", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                const char *p = str.TokenAtIdx(1, ' ');
                
                Log(P("Setting ID to \""), p, '"');

                wsprMessage_.SetId((char *)p);

                PrintCurrentValues();
            }
        });

        console_.RegisterCommand("altitude", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                uint32_t val = atol(str.TokenAtIdx(1, ' '));
                
                Log(P("Setting AltitudeFt to \""), val, '"');

                wsprMessage_.SetAltitudeFt(val);

                PrintCurrentValues();
            }
        });

        console_.RegisterCommand("speed", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                uint8_t val = atoi(str.TokenAtIdx(1, ' '));
                
                Log(P("Setting SpeedMPH to \""), val, '"');

                wsprMessage_.SetSpeedMph(val);

                PrintCurrentValues();
            }
        });

        console_.RegisterCommand("temp", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                int8_t val = atoi(str.TokenAtIdx(1, ' '));
                
                Log(P("Setting TemperatureC to \""), val, '"');

                wsprMessage_.SetTemperatureC(val);

                PrintCurrentValues();
            }
        });

        console_.RegisterCommand("mvolt", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                uint16_t val = atol(str.TokenAtIdx(1, ' '));
                
                Log(P("Setting MilliVolt to \""), val, '"');

                wsprMessage_.SetMilliVoltage(val);

                PrintCurrentValues();
            }
        });
        
        
        //////////////////////////////////////////////////////////////
        //
        // Radio controls
        //
        //////////////////////////////////////////////////////////////

        console_.RegisterCommand("on", [this](char *){
            Log(P("Radio On"));

            wsprMessageTransmitter_.SetCalibration(mtc_);

            wsprMessageTransmitter_.RadioOn();

            wsprMessageTransmitter_.SetFreqHundredths(freqInHundredths_);

            onOff_ = 1;
        });

        console_.RegisterCommand("freq", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                uint32_t val = atol(str.TokenAtIdx(1, ' '));

                freqInHundredths_ = val * 100;
                
                Log(P("Setting freq to \""), freqInHundredths_ / 100.0, '"');

                if (onOff_)
                {
                    wsprMessageTransmitter_.SetFreqHundredths(freqInHundredths_);
                }

                PrintCurrentValues();
            }
        });
        
        console_.RegisterCommand("chan", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                uint32_t chan = atol(str.TokenAtIdx(1, ' '));

                freqInHundredths_ =
                    (WSPRMessageTransmitter::WSPR_DEFAULT_DIAL_FREQ * 100UL) +
                    (WSPRMessageTransmitter::WSPR_OFFSET_FROM_DIAL_TO_USABLE_HZ * 100UL) +
                    (chan * WSPRMessageTransmitter::WSPR_CHANNEL_BANDWIDTH_HUNDREDTHS_HZ);

                Log(P("Setting channel to "), chan, P(", freq now "), freqInHundredths_ / 100.0);

                if (onOff_)
                {
                    wsprMessageTransmitter_.SetFreqHundredths(freqInHundredths_);
                }

                PrintCurrentValues();
            }
        });
        
        console_.RegisterCommand("send", [this](char *cmdStr){
            Str str(cmdStr);

            uint8_t type = 1;
            if (str.TokenCount(' ') == 2)
            {
                type = atol(str.TokenAtIdx(1, ' '));
            }
            
            Log(P("Sending type "), type);

            if (!onOff_)
            {
                console_.Exec("on");
            }

            PrintCurrentValues();
            
            wsprMessageTransmitter_.Send(&wsprMessage_, type);

            Log(P("Send complete"));
        });
        
        console_.RegisterCommand("off", [this](char *){
            Log(P("Radio Off"));

            wsprMessageTransmitter_.RadioOff();

            onOff_ = 0;
        });

        
        //////////////////////////////////////////////////////////////
        //
        // Temperature
        //
        //////////////////////////////////////////////////////////////

        console_.RegisterCommand("temp", [this](char *){
            Log(P("Radio Off"));

            wsprMessageTransmitter_.RadioOff();

            onOff_ = 0;
        });
        

        //////////////////////////////////////////////////////////////
        //
        // Tuning
        //
        //////////////////////////////////////////////////////////////

        console_.RegisterCommand("crystalCorrectionFactor", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                mtc_.crystalCorrectionFactor = atol(str.TokenAtIdx(1, ' '));
                
                Log(P("Setting crystalCorrectionFactor to "), mtc_.crystalCorrectionFactor);

                PrintCurrentValues();

                if (onOff_)
                {
                    console_.Exec("on");
                }
            }
        });
        
        console_.RegisterCommand("systemClockOffsetMs", [this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                mtc_.systemClockOffsetMs = atol(str.TokenAtIdx(1, ' '));
                
                Log(P("Setting systemClockOffsetMs to "), mtc_.systemClockOffsetMs);

                PrintCurrentValues();

                if (onOff_)
                {
                    console_.Exec("on");
                }
            }
        });

        console_.RegisterCommand("timeCalibrate", [this](char *){
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
        });
        
        console_.RegisterCommand("help", [this](char *){
            PrintMenu();
        });
    }
    
    void SetupInitialState()
    {
        // Set some defaults
        FillOutStandardWSPRMessage();
        
        // Override GPS-related due to not having a lock yet
        wsprMessage_.SetGrid("FN20XR");
        wsprMessage_.SetAltitudeFt(13500);
        wsprMessage_.SetSpeedMph(88);
        
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

    void PrintCurrentValues()
    {
        const char *callsign = NULL;
        const char *grid     = NULL;
        uint8_t     powerDbm = 0;
        
        wsprMessage_.GetData(callsign, grid, powerDbm);
        
        Log(P("Current Values"));
        Log(P("--------------"));
        Log(P("freq                   : "), freqInHundredths_ / 100.0);
        Log(P("callsign               : "), callsign);
        Log(P("grid                   : "), grid);
        Log(P("powerDbm               : "), powerDbm);
        Log(P("crystalCorrectionFactor: "), mtc_.crystalCorrectionFactor);
        Log(P("systemClockOffsetMs    : "), mtc_.systemClockOffsetMs);
    }

    void PrintMenu()
    {
        LogNL(4);
        Log(P("WSPR Calibrator"));
        Log(P("---------------"));
        LogNL();
        Log(P("Time Related"));
        LogX('-', 12);
        Log(P("timeCalibrate             - run time calibration"));
        Log(P("systemClockOffsetMs <val> - apply time calibration"));
        LogNL();
        //Log(P("Data Related"));
        //LogX('-', 20);
        //Log(P("callsign <callsign> - set callsign"));
        //Log(P("grid <grid> - set grid"));
        //Log(P("power <power> - set power"));
        //LogNL();
        Log(P("Frequency Related"));
        LogX('-', 17);
        Log(P("chan <num>                    - change WSPR frequency channel"));
        Log(P("crystalCorrectionFactor <val> - apply frequency calibration"));
        LogNL();
        Log(P("on  - Turn on radio"));
        Log(P("off - Turn off radio"));
        LogNL();
        Log(P("Other"));
        LogX('-', 5);
        //Log(P("test - test the configuration of the message to be sent"));
        Log(P("send - send message whose contents were configured"));
        Log(P("help - this menu"));
        LogNL();
        LogNL();
        PrintCurrentValues();
        LogNL();
    }


private:

    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    SerialAsyncConsoleEnhanced<25>  console_;
    
    WSPRMessageTransmitter::Calibration &mtc_;
    
    uint32_t freqInHundredths_;

    uint8_t onOff_;
};








#endif  // __APP_PICO_TRACKER_WSPR_1_TEST_H__





































