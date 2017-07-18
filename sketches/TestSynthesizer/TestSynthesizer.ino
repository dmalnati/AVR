#include "Synthesizer.h"


static Synthesizer s;


void Play(Synthesizer::Note n)
{
    s.OnKeyDown(n);
    PAL.Delay(800);
    
    s.OnKeyUp();
    PAL.Delay(200);
}

void setup()
{
    s.Init();
    
    s.Start();

    while (1)
    {
        Play(Synthesizer::Note::B);
        Play(Synthesizer::Note::A);
        Play(Synthesizer::Note::G);
        Play(Synthesizer::Note::A);
        Play(Synthesizer::Note::B);
        Play(Synthesizer::Note::B);
        Play(Synthesizer::Note::B);

        PAL.Delay(5000);
    }
}

void loop() {}


