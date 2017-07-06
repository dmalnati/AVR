#include "Evm.h"

#include "PinInputAnalog.h"

#include "SignalSourceSineWave.h"
#include "SignalDAC.h"
#include "Timer2.h"


class AnalogController
{
    uint16_t SAMPLE_RATE = 40000;
    
    uint16_t FREQ_LOW  =    1;
    uint16_t FREQ_HIGH = 600;

    uint8_t MIN_STEP_SIZE = 10;
    
public:
    AnalogController(uint8_t pin)
    : pia_(pin)
    {
        // Nothing to do
    }

    void Start()
    {
        Serial.begin(9600);
        
        // Register callbacks for changes in analog value
        pia_.SetCallback([this](uint16_t val){
            OnReading(val);
        });
        pia_.SetMinimumChange(MIN_STEP_SIZE);
        pia_.Enable();

        // Calibrate DAC parameters
        dac_.SetSampleRate(SAMPLE_RATE);
        DoFirstReading();

        // Start the noise
        dac_.Start();

        // Start the events
        evm_.MainLoop();
    }

private:

    void DoFirstReading()
    {
        uint16_t val = pia_.GetValue();

        OnReading(val);
    }

    void OnReading(uint16_t val)
    {
        // calculate frequency
        uint16_t frequency =
            FREQ_LOW + ((FREQ_HIGH - FREQ_LOW) * ((double)val / 1023.0));

        Serial.print("val: "); Serial.print(val);
        Serial.print("-> frequency: "); Serial.println(frequency);

        dac_.SetFrequency(frequency);
    }

    Evm::Instance<10,10,10> evm_;
    
    PinInputAnalog                           pia_;
    SignalDAC<SignalSourceSineWave, Timer2>  dac_;
};


static uint8_t PIN_INPUT = 28;

static AnalogController ac(PIN_INPUT);


void setup()
{
    ac.Start();
}


void loop() {}


















