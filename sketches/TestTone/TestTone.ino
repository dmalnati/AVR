#include "PAL.h"

const uint8_t PIN_SPEAKER = 15;
const uint8_t PIN_SPEAKER_ARDUINO = PAL.GetArduinoPinFromPhysicalPin(PIN_SPEAKER);

const uint32_t MAX_HZ = 65536;

static const uint32_t NOTE_C0 = 16;
static const uint32_t NOTE_D0 = 18;
static const uint32_t NOTE_E0 = 21;
static const uint32_t NOTE_F0 = 22;
static const uint32_t NOTE_G0 = 25;
static const uint32_t NOTE_A1 = 28;
static const uint32_t NOTE_B1 = 31;



struct NoteInfo
{
    NoteInfo(uint32_t noteFreqIn, uint32_t durationIn)
    : noteFreq(noteFreqIn)
    , duration(durationIn)
    {
        // Nothing to do
    }
    
    uint32_t noteFreq = 0;
    uint32_t duration = 1;
};


class Song
{
public:
    Song(NoteInfo *song, uint8_t songLen)
    : song_(song)
    , songLen_(songLen)
    {
        // nothing to do
    }

    NoteInfo *GetSongInfoList() { return song_; }
    uint8_t   GetSongInfoListLen() { return songLen_; }

private:
    NoteInfo *song_;
    uint8_t   songLen_;
};




void Play(uint32_t freq, uint32_t durationMs)
{
    tone(PIN_SPEAKER_ARDUINO, freq);

    PAL.Delay(durationMs);
}

void Silence(uint32_t durationMs)
{
    noTone(PIN_SPEAKER_ARDUINO);

    PAL.Delay(durationMs);
}







void PlaySong(Song s)
{
    while (1)
    {
        NoteInfo *song    = s.GetSongInfoList();
        uint8_t   songLen = s.GetSongInfoListLen();

        uint32_t durationMultiplierMs = 300;

        for (uint8_t i = 0; i < songLen; ++i)
        {
            NoteInfo ni = song[i];

            uint32_t freq      = ni.noteFreq;
            uint32_t durationMs = ni.duration * durationMultiplierMs;

            Play(freq, durationMs);
            Silence(50);
        }

        Silence(1000);
    }
}


void PlayMary()
{
    uint32_t NOTE_B = 247;  // B4
    uint32_t NOTE_A = 220;  // A4
    uint32_t NOTE_G = 196;  // G3
    uint32_t NOTE_D = 294;  // D4

    
    NoteInfo songMary[] = {
        { NOTE_B, 3 },
        { NOTE_A, 2 },
        { NOTE_G, 1 },
        { NOTE_A, 2 },
        { NOTE_B, 3 },
        { NOTE_B, 3 },
        { NOTE_B, 3 },

        { NOTE_A, 2 },
        { NOTE_A, 2 },
        { NOTE_A, 2 },
        { NOTE_B, 3 },
        { NOTE_D, 5 },
        { NOTE_D, 5 },

        { NOTE_B, 3 },
        { NOTE_A, 2 },
        { NOTE_G, 1 },
        { NOTE_A, 2 },
        { NOTE_B, 3 },
        { NOTE_B, 3 },
        { NOTE_B, 3 },

        { NOTE_A, 2 },
        { NOTE_A, 2 },
        { NOTE_B, 2 },
        { NOTE_A, 3 },
        { NOTE_G, 4 },
        { NOTE_G, 2 },
    };
    
    Song s(songMary, sizeof(songMary) / sizeof(NoteInfo));

    PlaySong(s);
}



void PlayAllFrequencies()
{
    for (uint32_t i = 0; i < MAX_HZ; ++i)
    {
        Play(i, 100);
    }
}


void PlayFullScale()
{
    uint32_t freqList[] = { NOTE_C0, NOTE_D0, NOTE_E0, NOTE_F0, NOTE_G0, NOTE_A1, NOTE_B1 };
    uint8_t  freqListLen = sizeof(freqList) / sizeof(uint32_t);

    uint8_t octaveLim = 8;
    
    while (1)
    {
        for (uint8_t octaveCount = 1; octaveCount <= octaveLim; ++octaveCount)
        {
            for (uint8_t i = 0; i < freqListLen; ++i)
            {
                Play((freqList[i] << octaveCount), 300);
            }
        }
    }
}


void PlayAFewNotes()
{
    while (1)
    {
        Play((NOTE_C0 << 1), 1000);
        Play((NOTE_C0 << 2), 1000);
        Play((NOTE_C0 << 3), 1000);
        Play((NOTE_C0 << 4), 1000);
        Play((NOTE_C0 << 5), 1000);
        Play((NOTE_C0 << 6), 1000);
        Play((NOTE_C0 << 7), 1000);
        Play((NOTE_C0 << 8), 1000);
    }
}


void setup()
{
    //PlayAFewNotes();
    //PlayFullScale();
    PlayMary();
}

void loop() {}


