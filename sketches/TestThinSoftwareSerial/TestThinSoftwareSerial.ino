#include "ThinSoftwareSerial.h"
#include "PAL.h"
#include "LogBlob.h"


static const int8_t PIN_GPS_RX = 23;
static const int8_t PIN_GPS_TX = 24;

static ThinSoftwareSerial ss(PIN_GPS_RX, PIN_GPS_TX);

void setup()
{
    LogStart(9600);
    Log("Starting");

    ss.begin(9600);
    //ss.listen();
    Log("SS listening");

    while (1)
    {
        uint8_t availCount = ss.available();

        if (availCount)
        {
//            Log("Avail: ", availCount);
          
            uint8_t buf[availCount + 1];

            for (uint8_t i = 0; i < availCount; ++i)
            {
                buf[i] = ss.read();
                if (isprint(buf[i]) || buf[i] == ' ' || buf[i] == '\n')
                {
                    if (buf[i] == '\r')
                    {
                        LogNNL('\n');
                    }
                    else
                    {
                        LogNNL((char)buf[i]);
                    }
                }
                else
                {
                    LogNNL('_');
                }

                if (buf[i] == '\n')
                {
                    break;
                }
            }

            buf[availCount] = '\0';

            //LogBlob(buf, availCount);
        }

        //PAL.Delay(50);
    }
}

void loop() {}
