#include "Evm.h"

#include "TimedEventHandler.h"

#include "Synthesizer.h"
#include "SynthesizerVoice.h"
#include "SynthesizerVoiceSerialInterface.h"


class Controller
{
public:
    Controller()
    : svsi_(s_.GetSynthesizerVoice(), &s_)
    {
        // Nothing to do
    }

    void PlayAsync(Synthesizer::Note n)
    {
        s_.OnKeyDown(n);

        tedKeyUp_.SetCallback([this](){
            s_.OnKeyUp();
        });
        tedKeyUp_.RegisterForTimedEvent(800);
    }

    void Start()
    {
        Serial.begin(9600);

        Serial.println("Starting");

        // Set up music player
        tedNextNote_.SetCallback([this](){
            static uint8_t noteListIdx = 0;
            static Synthesizer::Note noteList[] = {
                Synthesizer::Note::B,
                Synthesizer::Note::A,
                Synthesizer::Note::G,
                Synthesizer::Note::A,
                Synthesizer::Note::B,
                Synthesizer::Note::B,
                Synthesizer::Note::B
            };
            static uint8_t noteListLen = sizeof(noteList) / sizeof(Synthesizer::Note);

            Synthesizer::Note n = noteList[noteListIdx];
            ++noteListIdx;
            if (noteListIdx >= noteListLen)
            {
                noteListIdx = 0;
            }
            
            PlayAsync(n);
        });
        //tedNextNote_.RegisterForTimedEventInterval(1000);
        
        // Start up serial controller
        svsi_.Init();

        // Set up and start synthesizer
        s_.Init();
        s_.Start();

        // Start the events
        evm_.MainLoop();
    }

private:

    Evm::Instance<10,10,10> evm_;

    Synthesizer s_;
    SynthesizerVoiceSerialInterface<SynthesizerVoice<Timer2>> svsi_;

    TimedEventHandlerDelegate tedNextNote_;
    TimedEventHandlerDelegate tedKeyUp_;
};

static Controller c;


void setup()
{
    c.Start();
}


void loop() {}


















