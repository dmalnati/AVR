#ifndef __APP_MULTIMETER_H__
#define __APP_MULTIMETER_H__


#include "Eeprom.h"
#include "Evm.h"
#include "MuxAnalogDigitalCD74HC4067.h"
#include "Utl@fix@Serial.h"


struct AppMultimeterConfig
{
    // Pins used for multiplexer to indicate channel
    uint8_t pinMuxBit0;
    uint8_t pinMuxBit1;
    uint8_t pinMuxBit2;
    uint8_t pinMuxBit3;
    
    // Pin doing actual ADC
    uint8_t pinAdc;
};

class AppMultimeter
{
private:
    static const uint32_t DEFAULT_SAMPLE_RATE_MS = 1000;
    
    struct PersistentConfiguration
    {
        uint32_t SAMPLE_RATE_MS;
    };
    
private:
    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
public:
    AppMultimeter(AppMultimeterConfig &cfg)
    : cfg_(cfg)
    , adcMux_(cfg_.pinMuxBit0,
              cfg_.pinMuxBit1,
              cfg_.pinMuxBit2,
              cfg_.pinMuxBit3,
              cfg_.pinAdc)
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
        @fix@Serial.begin(9600);
        @fix@Serial.println(F("Starting"));
        
        // Set up sample rate based either on default or prior stored value
        LoadSampleRate();
        @fix@Serial.print("Sample rate: "); @fix@Serial.println(pc_.SAMPLE_RATE_MS);
        
        // Set up user input handling
        SetUpUserInputHandlers();
        
        // Set up reporting
        tedSample_.SetCallback([this](){ OnSample(); });
        tedSample_.RegisterForTimedEventInterval(GetSampleRate(), 0);
        
        // Start CSV output
        PrintHeader();
        
        // Handle events
        evm_.MainLoop();
    }
    
private:

    void LoadSampleRate()
    {
        SetSampleRate(GetSampleRate());
    }

    void SetSampleRate(uint32_t sampleRateMs)
    {
        pc_.SAMPLE_RATE_MS = sampleRateMs;
        
        eaPc_.Write(pc_);
    }
    
    uint32_t GetSampleRate()
    {
        uint32_t retVal = DEFAULT_SAMPLE_RATE_MS;
        
        if (eaPc_.Read(pc_))
        {
            retVal = pc_.SAMPLE_RATE_MS;
        }
        
        return retVal;
    }
    
    void PrintHeader()
    {
        // print header
        const char *sep = "";
        for (uint8_t i = 0; i < 16; ++i)
        {
            @fix@Serial.print(sep);
            @fix@Serial.print("CH"); @fix@Serial.print(i);
            
            sep = ",";
        }
        @fix@Serial.println();
    }

    void OnSample()
    {
        // print data
        const char *sep = "";
        for (uint8_t i = 0; i < 16; ++i)
        {
            uint16_t adcVal = adcMux_.AnalogReadChannel(i);
            
            @fix@Serial.print(sep);
            @fix@Serial.print(adcVal);
            
            sep = ",";
        }
        @fix@Serial.println();
    }

    void SetUpUserInputHandlers()
    {
        console_.RegisterCommand("sampleRateMs", [this](char *cmdStr) {
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 2)
            {
                uint32_t sampleRateMs = atol(str.TokenAtIdx(1, ' '));
                
                PrintHeader();
                
                SetSampleRate(sampleRateMs);
                tedSample_.RegisterForTimedEventInterval(GetSampleRate(), 0);
            }
        });
        
        console_.SetVerbose(0);
        
        console_.Start();
    }

private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppMultimeterConfig &cfg_;
    
    PersistentConfiguration                   pc_;
    EepromAccessor<PersistentConfiguration> eaPc_;
    
    TimedEventHandlerDelegate tedSample_;
    
    MuxAnalogDigitalCD74HC4067 adcMux_;
    
    static const uint8_t NUM_COMMANDS = 1;
    SerialAsyncConsoleEnhanced<NUM_COMMANDS> console_;
};


#endif  // __APP_MULTIMETER_H__




























