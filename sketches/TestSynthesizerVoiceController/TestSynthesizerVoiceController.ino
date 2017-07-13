#include "Evm.h"

#include "Timer2.h"
#include "PinInputAnalog.h"
#include "SynthesizerVoice.h"
#include "SynthesizerVoiceSerialInterface.h"


class AnalogController
{
    //uint16_t SAMPLE_RATE = 40000;
    //uint16_t SAMPLE_RATE = 20000;
    uint16_t SAMPLE_RATE = 10000;
    //uint16_t SAMPLE_RATE = 4000;
    //uint16_t SAMPLE_RATE = 65000;

    uint16_t FREQ_LOW  =    1;
    uint16_t FREQ_HIGH = 600;

    uint8_t MIN_STEP_SIZE = 10;

    static const uint16_t TONE_DURATION_MS = 500;
    
public:
    AnalogController(uint8_t pin)
    : pia_(pin)
    , svsi_(&sv_)
    {
        // Nothing to do
    }

    void Start()
    {
        Serial.begin(9600);

        Serial.println("Starting");
        
        // Register callbacks for changes in analog value
        pia_.SetCallback([this](uint16_t val){
            OnReading(val);
        });
        pia_.SetMinimumChange(MIN_STEP_SIZE);
        pia_.Enable();

        // Start up serial controller
        svsi_.Init();

        // Calibrate SynthVoice parameters
        sv_.SetSampleRate(SAMPLE_RATE);

        // Get first analog reading
        //DoFirstReading();

        // Start the noise
        sv_.Start();

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

        //sv_.SetFrequency(frequency);
        sv_.StartNote(frequency, TONE_DURATION_MS);
    }

    Evm::Instance<10,10,10> evm_;
    
    PinInputAnalog  pia_;

    SynthesizerVoice<Timer2>  sv_;
    SynthesizerVoiceSerialInterface<SynthesizerVoice<Timer2>> svsi_;
};


static uint8_t PIN_INPUT = 28;

static AnalogController ac(PIN_INPUT);


void setup()
{
    ac.Start();
}


void loop() {}


















