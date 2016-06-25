#include "UtlStreamBlob.h"

static const char    *str6      = "heynow";
static const char    *str8      = "whats up";
static const char    *str9      = "niiiiice!";
static       uint8_t  blob[256] = { 0 };

void setup()
{
    for (uint16_t i = 0; i <= 255; ++i)
    {
        blob[i] = (uint8_t)i;
    }

    Serial.begin(9600);
}


void ShowHexOnly()
{
    Serial.println("=== HEX ONLY ===");
    Show(0, 1);
}

void ShowBinOnly()
{
    Serial.println("=== BINARY ONLY ===");
    Show(1, 0);
}

void ShowBoth()
{
    Serial.println("=== HEX AND BINARY ===");
    Show(1, 1);
}

void ShowNeither()
{
    Serial.println("=== NEITHER HEX NOR BINARY ===");
    Show(0, 0);
}

void Show(uint8_t showBin, uint8_t showHex)
{
    Serial.println("str6");
    StreamBlob(Serial, (uint8_t *)str6, strlen(str6), showBin, showHex);
    Serial.println();

    Serial.println("str8");
    StreamBlob(Serial, (uint8_t *)str8, strlen(str8), showBin, showHex);
    Serial.println();

    Serial.println("str9");
    StreamBlob(Serial, (uint8_t *)str9, strlen(str9), showBin, showHex);
    Serial.println();

    Serial.println("Blob of every byte value");
    StreamBlob(Serial, (uint8_t *)&blob, sizeof(blob), showBin, showHex);
    Serial.println();
}

void loop()
{
    ShowHexOnly();
    ShowBinOnly();
    ShowBoth();
    ShowNeither();
}












