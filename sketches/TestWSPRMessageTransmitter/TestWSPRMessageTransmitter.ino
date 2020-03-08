#include "Evm.h"
#include "Log.h"
#include "SerialInput.h"
#include "WSPRMessageTransmitter.h"
#include "WSPRMessagePicoTrackerWSPR1.h"

static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;
static SerialAsyncConsoleEnhanced<25>  console;

static const uint8_t pinB = 25;

static WSPRMessage                          mOld;
static WSPRMessagePicoTrackerWSPR1          m;
static WSPRMessageTransmitter               mt;
static WSPRMessageTransmitter::Calibration  mtc;
static uint32_t freqInHundredths = mt.GetCalculatedFreqHundredths();

static uint8_t onOff = 0;


void PrintCurrentValues()
{
    const char *callsign = NULL;
    const char *grid     = NULL;
    uint8_t     powerDbm = 0;
    
    m.GetData(callsign, grid, powerDbm);
    
    Log(P("Current Values"));
    Log(P("--------------"));
    Log(P("freq                   : "), freqInHundredths / 100.0);
    Log(P("callsign               : "), callsign);
    Log(P("grid                   : "), grid);
    Log(P("powerDbm               : "), powerDbm);
    Log(P("crystalCorrectionFactor: "), mtc.crystalCorrectionFactor);
    Log(P("systemClockOffsetMs    : "), mtc.systemClockOffsetMs);
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

void setup()
{
    LogStart(9600);
    
    //////////////////////////////////////////////////////////////
    //
    // Traditional WSPR message settings
    //
    //////////////////////////////////////////////////////////////

    console.RegisterCommand("callsign", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');
            
            Log(P("Setting callsign to \""), p, '"');

            m.SetCallsign((char *)p);

            PrintCurrentValues();
        }
    });

    console.RegisterCommand("grid", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');
            
            Log(P("Setting grid to \""), p, '"');

            m.SetGrid((char *)p);

            PrintCurrentValues();
        }
    });
    
    console.RegisterCommand("power", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint8_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Setting power to \""), val, '"');

            m.SetPower(val);

            PrintCurrentValues();
        }
    });
    
    //////////////////////////////////////////////////////////////
    //
    // Encoded WSPR message settings
    //
    //////////////////////////////////////////////////////////////

    console.RegisterCommand("id", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');
            
            Log(P("Setting ID to \""), p, '"');

            m.SetId((char *)p);

            PrintCurrentValues();
        }
    });

    console.RegisterCommand("altitude", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));
            
            Log(P("Setting AltitudeFt to \""), val, '"');

            m.SetAltitudeFt(val);

            PrintCurrentValues();
        }
    });

    console.RegisterCommand("speed", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint8_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Setting SpeedMPH to \""), val, '"');

            //m.SetSpeedMph(val);

            PrintCurrentValues();
        }
    });

    console.RegisterCommand("temp", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            int8_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log(P("Setting TemperatureC to \""), val, '"');

            m.SetTemperatureC(val);

            PrintCurrentValues();
        }
    });

    #if 1
    console.RegisterCommand("mvolt", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint16_t val = atol(str.TokenAtIdx(1, ' '));
            
            Log(P("Setting MilliVolt to \""), val, '"');

            m.SetMilliVoltage(val);

            PrintCurrentValues();
        }
    });
    #endif
    
    
    //////////////////////////////////////////////////////////////
    //
    // Radio controls
    //
    //////////////////////////////////////////////////////////////

    console.RegisterCommand("on", [](char *){
        Log(P("Radio On"));

        mt.SetCalibration(mtc);

        mt.RadioOn();

        mt.SetFreqHundredths(freqInHundredths);

        onOff = 1;
    });

    console.RegisterCommand("freq", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));

            freqInHundredths = val * 100;
            
            Log(P("Setting freq to \""), freqInHundredths / 100.0, '"');

            if (onOff)
            {
                mt.SetFreqHundredths(freqInHundredths);
            }

            PrintCurrentValues();
        }
    });
    
    console.RegisterCommand("chan", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t chan = atol(str.TokenAtIdx(1, ' '));

            freqInHundredths =
                (WSPRMessageTransmitter::WSPR_DEFAULT_DIAL_FREQ * 100UL) +
                (WSPRMessageTransmitter::WSPR_OFFSET_FROM_DIAL_TO_USABLE_HZ * 100UL) +
                (chan * WSPRMessageTransmitter::WSPR_CHANNEL_BANDWIDTH_HUNDREDTHS_HZ);

            Log(P("Setting channel to "), chan, P(", freq now "), freqInHundredths / 100.0);

            if (onOff)
            {
                mt.SetFreqHundredths(freqInHundredths);
            }

            PrintCurrentValues();
        }
    });
    
    console.RegisterCommand("send", [](char *cmdStr){
        Str str(cmdStr);

        uint8_t type = 1;
        if (str.TokenCount(' ') == 2)
        {
            type = atol(str.TokenAtIdx(1, ' '));
        }
        
        Log(P("Sending type "), type);

        if (!onOff)
        {
            console.Exec("on");
        }

        PrintCurrentValues();
        
        //mt.Send(&m, type);
        //mt.Send(&mOld, type);

        Log(P("Send complete"));
    });
    
    console.RegisterCommand("off", [](char *){
        Log(P("Radio Off"));

        mt.RadioOff();

        onOff = 0;
    });

    
    //////////////////////////////////////////////////////////////
    //
    // Tuning
    //
    //////////////////////////////////////////////////////////////

    console.RegisterCommand("crystalCorrectionFactor", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            mtc.crystalCorrectionFactor = atol(str.TokenAtIdx(1, ' '));
            
            Log(P("Setting crystalCorrectionFactor to "), mtc.crystalCorrectionFactor);

            PrintCurrentValues();

            if (onOff)
            {
                console.Exec("on");
            }
        }
    });
    
    console.RegisterCommand("systemClockOffsetMs", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            mtc.systemClockOffsetMs = atol(str.TokenAtIdx(1, ' '));
            
            Log(P("Setting systemClockOffsetMs to "), mtc.systemClockOffsetMs);

            PrintCurrentValues();

            if (onOff)
            {
                console.Exec("on");
            }
        }
    });

    console.RegisterCommand("timeCalibrate", [](char *){
        PAL.PinMode(pinB, OUTPUT);

        Log(P("Testing system clock"));
        Log(P("Current systemClockOffsetMs = "), mtc.systemClockOffsetMs);
        Log(P("Measure with scope, get pulse widths to "), WSPRMessageTransmitter::WSPR_DELAY_MS, P(" ms"));
        Log(P("Positive adjustments make pulses smaller"));
        Log(P("negative adjustments make pulses larger"));
        Log(P("Starting test"));

        for (uint8_t i = 0; i < 3; ++i)
        {
            PAL.DigitalWrite(pinB, HIGH);
            PAL.Delay(WSPRMessageTransmitter::WSPR_DELAY_MS - mtc.systemClockOffsetMs);
            PAL.DigitalWrite(pinB, LOW);
            PAL.Delay(WSPRMessageTransmitter::WSPR_DELAY_MS - mtc.systemClockOffsetMs);
        }

        Log(P("Done"));
    });
    
    console.RegisterCommand("help", [](char *){
        PrintMenu();
    });


    // Set some defaults
    //m.SetCallsign("KD2KDD");
    //m.SetPower(27);
    m.SetId("Q1");
    m.SetGrid("FN20XR");
    m.SetAltitudeFt(13500);
    //m.SetSpeedMph(88);
    m.SetTemperatureC(-30);
    m.SetMilliVoltage(3100);

    // Toggle pinB whenever a bit changes
    mt.SetCallbackOnBitChange([](){
        PAL.PinMode(pinB, OUTPUT);
        PAL.DigitalToggle(pinB);
    });

    console.SetVerbose(0);
    console.Start();
    console.Exec("pin set 15 1");
    console.Exec("help");

    evm.MainLoop();
}

void loop() {}
