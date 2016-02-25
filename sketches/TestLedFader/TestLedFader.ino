#include <Evm.h>
#include <LedFader.h>
#include <InterruptEventHandler.h>


class FaderToggler
: private InterruptEventHandler
{
public:
    FaderToggler(LEDFaderControl *fc)
    : fc_(fc)
    , onOff_(1)
    {
        // nothing to do
    }

    void StartMonitoring(uint8_t pin)
    {
        RegisterForInterruptEvent(pin);
    }

    virtual void OnInterruptEvent(uint8_t logicLevel)
    {
        if (onOff_ == 1)
        {
            fc_->Stop();

            onOff_ = 0;
        }
        else // (onOff_ == 0)
        {
            fc_->FadeForever();

            onOff_ = 1;
        }
    }
    
private:
    LEDFaderControl *fc_;
    uint8_t          onOff_;
};


// Set up configuration
static uint8_t pinRed   = 15;
static uint8_t pinGreen = 16;
static uint8_t pinBlue  = 17;

static uint8_t pinFtt = 12;
static uint8_t pinYes = 11;
static uint8_t pinNo  =  5;

static uint8_t pinAttentionButton = 18;
static uint8_t pinYesButton       =  6;
static uint8_t pinClearButton     = 27;

// Calculate sizing values for Evm and get instance
static const uint8_t C_IDLE  = 3;
static const uint8_t C_TIMED = 1;
static const uint8_t C_INTER = 3;

static Evm::Instance<C_IDLE,C_TIMED,C_INTER> evm;

// Create objects which rely on Evm::GetInstance() working
static LEDFaderRGB<1> ledFaderRGB;
static LEDFader<1,1>  ledFader1;
static LEDFader<2,2>  ledFader2;

static FaderToggler ft1(&ledFaderRGB);
static FaderToggler ft2(&ledFader1);
static FaderToggler ft3(&ledFader2);


void loop()
{
    // Get objects configured
    ledFaderRGB.AddLED(pinRed, pinGreen, pinBlue);
    ledFaderRGB.FadeForever();

    ledFader1.AddLED(pinYes);
    ledFader1.FadeForever();

    ledFader2.AddLED(pinFtt,  45);
    ledFader2.AddLED(pinNo,  135);
    ledFader2.FadeForever(2000);

    ft1.StartMonitoring(pinAttentionButton);
    ft2.StartMonitoring(pinYesButton);
    ft3.StartMonitoring(pinClearButton);
    


    Evm::GetInstance().MainLoop();  // this works
    //evm.MainLoop();   // this also works
}



void setup()
{
}
