#include "PAL.h"

#include "SignalSourceSineWave.h"
#include "SignalDAC.h"
#include "Timer2.h"


// Adapted from:
// http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March

#define c 261
#define d 294
#define e 329
#define f 349
#define g 391
#define gS 415
#define a 440
#define aS 455
#define b 466
#define cH 523
#define cSH 554
#define dH 587
#define dSH 622
#define eH 659
#define fH 698
#define fSH 740
#define gH 784
#define gSH 830
#define aH 880


static SignalDAC<SignalSourceSineWave, Timer2> dac;


void setup()
{
    dac.SetSampleRate(30000);
    
    while (1)
    {
        PlayMusic();

        PAL.Delay(2000);
    }
}


void beep(uint16_t frequency, uint16_t durationMs)
{
    // silent period before each note
    dac.Suspend();
    PAL.Delay(20);

    // play the intended note for the given duration
    dac.SetFrequency(frequency);
    dac.UnSuspend();
    PAL.Delay(durationMs);
}


void PlayMusic()
{
    dac.Start();
    
    beep(a, 500);
    beep(a, 500);
    beep(a, 500);
    beep(f, 350);
    beep(cH, 150);
    beep(a, 500);
    beep(f, 350);
    beep(cH, 150);
    beep(a, 650);
 
    PAL.Delay(150);
    //end of first bit
 
    beep(eH, 500);
    beep(eH, 500);
    beep(eH, 500);
    beep(fH, 350);
    beep(cH, 150);
    beep(gS, 500);
    beep(f, 350);
    beep(cH, 150);
    beep(a, 650);
 
    PAL.Delay(150);
    //end of second bit...
 
    beep(aH, 500);
    beep(a, 300);
    beep(a, 150);
    beep(aH, 400);
    beep(gSH, 200);
    beep(gH, 200);
    beep(fSH, 125);
    beep(fH, 125);
    beep(fSH, 250);
 
    PAL.Delay(250);
 
    beep(aS, 250);
    beep(dSH, 400);
    beep(dH, 200);
    beep(cSH, 200);
    beep(cH, 125);
    beep(b, 125);
    beep(cH, 250);
 
    PAL.Delay(250);
 
    beep(f, 125);
    beep(gS, 500);
    beep(f, 375);
    beep(a, 125);
    beep(cH, 500);
    beep(a, 375);
    beep(cH, 125);
    beep(eH, 650);
 
    //end of third bit... (Though it doesn't play well)
    //let's repeat it
 
    beep(aH, 500);
    beep(a, 300);
    beep(a, 150);
    beep(aH, 400);
    beep(gSH, 200);
    beep(gH, 200);
    beep(fSH, 125);
    beep(fH, 125);
    beep(fSH, 250);
 
    PAL.Delay(250);
 
    beep(aS, 250);
    beep(dSH, 400);
    beep(dH, 200);
    beep(cSH, 200);
    beep(cH, 125);
    beep(b, 125);
    beep(cH, 250);
 
    PAL.Delay(250);
 
    beep(f, 250);
    beep(gS, 500);
    beep(f, 375);
    beep(cH, 125);
    beep(a, 500);
    beep(f, 375);
    beep(cH, 125);
    beep(a, 650);

    dac.Stop();
}




void loop() {}









