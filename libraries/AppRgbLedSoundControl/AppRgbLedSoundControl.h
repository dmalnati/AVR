#ifndef __APP_RGB_LED_SOUND_CONTROL_H__
#define __APP_RGB_LED_SOUND_CONTROL_H__


#include "Evm.h"
#include "IdleTimeEventHandler.h"
#include "Timer1.h"
#include "Timer2.h"
#include "SignalProcessingFHT.h"


struct AppRgbLedSoundControlConfig
{
    // Red   LED driven by OC1A - Pin 15
    // Green LED driven by OC1B - Pin 16
    // Blue  LED driven by OC2A - Pin 17
    
    // Analog input
    uint8_t pinMicAnalogInput;
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
    , pinMicAnalogInput_(cfg_.pinMicAnalogInput)
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
        timerAdcFht_.SetCallback([this](){
            OnDoAdcFht();
        });
        timerAdcFht_.RegisterForIdleTimeEvent();
    }
    
    void OnDoAdcFht()
    {
        SignalProcessingFHT::Measurement m;

        if (fht_.GetMeasurement(pinMicAnalogInput_, &m))
        {
            /*
            MoveCursorToUpperLeft();
            AnalyzeRange(m.valList,  2, 16);
            AnalyzeRange(m.valList,  8, 20);
            AnalyzeRange(m.valList, 16, 24);
            Serial.println();
            */
            
            OnFhtDataReadNew(m.valList);
            //DumpTable(m.valList, 48);
        }
    }
    
    void MoveCursorToUpperLeft()
    {
        const uint8_t esc = 0x1B;
        Serial.write(esc);
        Serial.print("[0;0f");
    }
    
    void AnalyzeRange(uint16_t *fhtValList,
                      uint16_t  idxStart,
                      uint16_t  idxLim)
    {
        uint16_t count = 0;
        for (uint16_t i = idxStart; i < idxLim; ++i)
        {
            count += fhtValList[i];
        }
        
        Serial.print("Range: ");
        Serial.print(idxStart);
        Serial.print(" - ");
        Serial.print(idxLim);
        Serial.print(" : ");
        Serial.print(count);
        Serial.print("        ");
        Serial.println();
    }
    
    void DumpTable(uint16_t *fthValList, uint16_t idxLim)
    {
        for (uint16_t i = 0; i < idxLim; ++i)
        {
            Serial.print(i);
            Serial.print(": ");
            Serial.println(fthValList[i]);
        }
        Serial.println();
    }
    
    void OnFhtDataReadNew(uint16_t *fhtValList)
    {
        uint8_t valRed   = CalculateRange(fhtValList,  2, 16);
        uint8_t valBlue  = CalculateRange(fhtValList,  8, 20);
        uint8_t valGreen = CalculateRange(fhtValList, 12, 24);
        
        OnNewValue(valRed,   tcLedRed_,   1);
        OnNewValue(valBlue,  tcLedBlue_,  2);
        OnNewValue(valGreen, tcLedGreen_, 4);
        
        /*
        tcLedRed_->SetValue(valRed);
        tcLedBlue_->SetValue(valBlue);
        tcLedGreen_->SetValue(valGreen);
        */
        
        // if one isn't at the max, then scale it to at least 3/4 max and
        // scale the others to it?
    }
    
    void OnNewValue(uint8_t val, TimerChannel *tc, uint8_t stepDown)
    {
        int16_t curValBig  = (int16_t)tc->GetValue();
        int16_t thisValBig = val;
        
        if (thisValBig < curValBig)
        {
            //static const uint8_t DEFAULT_STEP_DOWN_VAL = 5;
            //static const uint8_t DEFAULT_STEP_DOWN_VAL = 1;
            uint8_t DEFAULT_STEP_DOWN_VAL = stepDown;
            
            
            uint16_t stepDownVal =
                (curValBig - DEFAULT_STEP_DOWN_VAL) < thisValBig ?
                    curValBig - thisValBig                       :
                    DEFAULT_STEP_DOWN_VAL;
            
            
            //uint16_t stepDownVal = 1;
            
            
            //Serial.print("DN: ");
            //Serial.print(curValBig);
            //Serial.print(" -> ");
            
            curValBig -= stepDownVal;
            
            //Serial.print(curValBig);
            //Serial.println();
            
            
            // set curVal active
            tc->SetValue(curValBig);
        }
        else
        {
            // set immediately, higher values always are immediate
            
            /*
            Serial.print("UP: ");
            Serial.print(curValBig);
            Serial.print(" -> ");
            Serial.print(thisValBig);
            Serial.println();
            */
            
            tc->SetValue(thisValBig);
        }
    }
    
    uint8_t CalculateRange(uint16_t *fhtValList,
                           uint16_t  idxStart,
                           uint16_t  idxLim)
    {
        static const uint8_t MIN_VALUE =  10;
        static const uint8_t MAX_VALUE = 255;
        
        uint8_t retVal = 0;
        
        uint16_t bucketsUsed = 0;
        
        uint16_t count = 0;
        for (uint16_t i = idxStart; i < idxLim; ++i)
        {
            if (MIN_VALUE < fhtValList[i])
            {
                ++bucketsUsed;
                
                count += fhtValList[i];
            }
        }
        
        // Algo 1 -- trimmed count
        retVal = (count <= MAX_VALUE) ? count : MAX_VALUE;
        
        return retVal;
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
    
    // Event manager
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    // Config
    AppRgbLedSoundControlConfig cfg_;
    
    // Signal processor
    SignalProcessingFHT fht_;
    
    // Analog input
    Pin pinMicAnalogInput_;
    
    // Timer channels for each color
    TimerChannel *tcLedRed_;
    TimerChannel *tcLedGreen_;
    TimerChannel *tcLedBlue_;
    
    // Some time keeping
    IdleTimeEventHandlerDelegate timerAdcFht_;
};


#endif  // __APP_RGB_LED_SOUND_CONTROL_H__

















