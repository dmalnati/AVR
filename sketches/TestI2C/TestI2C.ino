#include "Log.h"
#include "PAL.h"
#include "I2C.h"


void ScanRange()
{
    const char *hexList = "0123456789ABCDEF";

    // Print header row
    LogNNL(" ");
    for (uint8_t i = 0; i < 16; ++i)
    {
        LogNNL(hexList[i]);
    }
    LogNL();

    // For each row, print prefix byte, and check the addr
    for (uint8_t i = 0; i <= 127; i += 16)
    {
        LogNNL(hexList[i / 16]);
        
        for (uint8_t j = 0; j < 16; ++j)
        {
            LogNNL(I2C.SlaveAddrPresent(i + j) ? "X" : " ");
        }

        LogNL();
    }
    LogNL();
}

void setup()
{
    LogStart(9600);
    Log("Starting");

    while (1)
    {
        Log("First addr found: ", I2C.GetFirstSlaveAddrPresent());
        LogNL();
        
        ScanRange();

        LogNL();
        
        PAL.Delay(2000);
    }
}

void loop() {}



