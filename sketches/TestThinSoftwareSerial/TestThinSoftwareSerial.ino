#include "ThinSoftwareSerial.h"
#include "PAL.h"
#include "LogBlob.h"


static const int8_t PIN_GPS_RX = 23;
static const int8_t PIN_GPS_TX = 24;

static ThinSoftwareSerial ss(PAL.GetArduinoPinFromPhysicalPin(PIN_GPS_RX),
                             PAL.GetArduinoPinFromPhysicalPin(PIN_GPS_TX));

void setup()
{
    LogStart(9600);
    Log("Starting");

    ss.begin(9600);
    ss.listen();
    Log("SS listening");

    while (1)
    {
        uint8_t availCount = ss.available();

        if (availCount)
        {
            Log("Avail: ", availCount);
          
            uint8_t buf[availCount + 1];

            for (uint8_t i = 0; i < availCount; ++i)
            {
                buf[i] = ss.read();
            }

            buf[availCount] = '\0';

            LogBlob(buf, availCount, 1, 1);
        }

        PAL.Delay(100);
    }
}

void loop() {}
