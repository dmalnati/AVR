#ifndef __APP_RGB_LED_SOUND_CONTROL_H__
#define __APP_RGB_LED_SOUND_CONTROL_H__


#include "Evm.h"
#include "IdleTimeEventHandler.h"
#include "Timer1.h"
#include "Timer2.h"

// Include FHT libraries, which are sensitive to preprocessor defs
#define FHT_N   256
#define LIN_OUT   1
#include "FHT.h"


struct AppRgbLedSoundControlConfig
{
    // Red   LED driven by OC1A - Pin 15
    // Green LED driven by OC1B - Pin 16
    // Blue  LED driven by OC2A - Pin 17
    
    // Analog input on ADC0
};


class AppRgbLedSoundControl
{
private:
    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;
    
public:

    AppRgbLedSoundControl(AppRgbLedSoundControlConfig &cfg)
    : cfg_(cfg)
    , tcLedRed_(Timer1::GetTimerChannelA())
    , tcLedGreen_(Timer1::GetTimerChannelB())
    , tcLedBlue_(Timer2::GetTimerChannelA())

    {
        // Nothing to do
    }
    
    ~AppRgbLedSoundControl()
    {
        // Nothing to do
    }
    
    void Run()
    {
        // This app is more about learning than being an actual functional
        // standalone device.  Serial output will be used unconditionally.
        Serial.begin(9600);
        
        Serial.println("Starting AppRgbLedSoundControl");
        
        // Set up operations
        SetUpAnalogInputs();
        SetUpLedControllingTimers();
        
        // Go
        evm_.MainLoop();
    }

private:

    void SetUpAnalogInputs()
    {
        // Set up ADC
        //TIMSK0 = 0; // turn off timer0 for lower jitter
        ADCSRA = 0xe5; // set the adc to free running mode
        ADMUX = 0x40; // use adc0
        DIDR0 = 0x01; // turn off the digital input for adc0
        
        timerAdcFht_.SetCallback([this](){
            OnDoAdcFht();
        });
        timerAdcFht_.RegisterForIdleTimeEvent();
    }
    
    void OnDoAdcFht()
    {
        // Collect samples
        for (uint16_t i = 0; i < FHT_N; ++i)
        {
            while(!(ADCSRA & 0x10)); // wait for adc to be ready
            ADCSRA = 0xf5; // restart adc
            byte m = ADCL; // fetch adc data
            byte j = ADCH;
            int k = (j << 8) | m; // form into an int
            k -= 0x0200; // form into a signed int
            k <<= 6; // form into a 16b signed int
            fht_input[i] = k; // put real data into bins
        }

        // Process samples
        fht_window(); // window the data for better frequency response
        fht_reorder(); // reorder the data before doing the fht
        fht_run(); // process the data in the fht
        fht_mag_lin(); // take the output of the fht
        
        OnFhtDataRead(fht_lin_out);
        
        //DumpTable(fht_lin_out);
    }
    
    void DumpTable(uint16_t *fthValList)
    {
        for (uint16_t i = 0; i < FHT_N/2; ++i)
        {
            Serial.print(i);
            Serial.print(": ");
            Serial.println(fthValList[i]);
        }
        Serial.println();
    }
    
    void OnFhtDataRead(uint16_t *fhtValList)
    {
        static const uint16_t MIN_VALUE = 10;
        static const uint16_t MAX_VALUE = 255;

        // Figure out Red channel
        uint8_t countRed = 0;
        for (uint8_t i = 0; i < 16; ++i)
        {
            if (MIN_VALUE < fhtValList[i] && fhtValList[i] < MAX_VALUE)
            {
                countRed += fhtValList[i];
            }
        }
        
        // Figure out Green channel
        uint8_t countGreen = 0;
        //for (uint8_t i = 32; i < 48; ++i)
        for (uint8_t i = 16; i < 32; ++i)
        {
            if (MIN_VALUE < fhtValList[i] && fhtValList[i] < MAX_VALUE)
            {
                countGreen += fhtValList[i];
            }
        }
        
        // Figure out Blue channel
        uint8_t countBlue = 0;
        //for (uint8_t i = 16; i < 32; ++i)
        for (uint8_t i = 32; i < 48; ++i)
        {
            if (MIN_VALUE < fhtValList[i] && fhtValList[i] < MAX_VALUE)
            {
                countBlue += fhtValList[i];
            }
        }

        // Figure which is higher
        uint8_t highest = 0;
        
        if (countRed   > highest) { highest = countRed;   }
        if (countGreen > highest) { highest = countGreen; }
        if (countBlue  > highest) { highest = countBlue;  }
        
        // Scale all others by the highest, to 255
        uint8_t valRed   = (double)countRed   * 255.0 / (double)highest;
        uint8_t valGreen = (double)countGreen * 255.0 / (double)highest;
        uint8_t valBlue  = (double)countBlue  * 255.0 / (double)highest;
        
        /*
        Serial.print("countRed  : "); Serial.println(countRed);
        Serial.print("countGreen: "); Serial.println(countGreen);
        Serial.print("countBlue : "); Serial.println(countBlue);
        Serial.print("valRed    : "); Serial.println(valRed);
        Serial.print("valGreen  : "); Serial.println(valGreen);
        Serial.print("valBlue   : "); Serial.println(valBlue);
        */
        
        // Set values
        tcLedRed_->SetValue(valRed);
        tcLedBlue_->SetValue(valBlue);
        tcLedGreen_->SetValue(valGreen);
    }
    

    void SetUpLedControllingTimers()
    {
        // Set up timers for fast wrapping, we just want high-res pwm
        Timer1::SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
        Timer1::SetTimerMode(Timer1::TimerMode::FAST_PWM_8_BIT);
        Timer1::SetTimerValue(0);
        
        Timer2::SetTimerPrescaler(Timer2::TimerPrescaler::DIV_BY_1);
        Timer2::SetTimerMode(Timer2::TimerMode::FAST_PWM);
        Timer2::SetTimerValue(0);
        
        // Set up Timer1 Channel A -- Red
        tcLedRed_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
        tcLedRed_->SetValue(0);
        
        // Set up Timer1 Channel B -- Green
        tcLedGreen_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
        tcLedGreen_->SetValue(0);
        
        // Set up Timer2 Channel A -- Blue
        tcLedBlue_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::CLEAR);
        tcLedBlue_->SetValue(0);
        
        // Begin timer operation
        Timer1::StartTimer();
        Timer2::StartTimer();
    }
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppRgbLedSoundControlConfig cfg_;
    
    // Red
    TimerChannel *tcLedRed_;
    
    // Green
    TimerChannel *tcLedGreen_;
    
    // Blue
    TimerChannel *tcLedBlue_;
    
    IdleTimeEventHandlerDelegate timerAdcFht_;
};


#endif  // __APP_RGB_LED_SOUND_CONTROL_H__

















