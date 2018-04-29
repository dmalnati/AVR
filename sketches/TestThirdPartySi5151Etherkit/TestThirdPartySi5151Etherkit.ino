#include <si5351.h>
#include "Wire.h"


static Si5351 si5351;


void setup()
{
  @fix@Serial.begin(9600);
  @fix@Serial.println("Starting");

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_6MA);

  while (1)
  {
    uint32_t freqLow  = 10000000;
    uint32_t freqHigh = 11000000;
    uint32_t freqStep =    50000;

    uint32_t stepDelayMs = 100;

    si5351.output_enable(SI5351_CLK0, 1);

    // go up the range
    for (uint32_t freq = freqLow; freq <= freqHigh; freq += freqStep)
    {
        si5351.set_freq((uint64_t)freq * (uint64_t)100ULL, SI5351_CLK0);
        
        delay(stepDelayMs);
    }

    // go down the range
    for (uint32_t freq = freqHigh; freq >= freqLow; freq -= freqStep)
    {
        si5351.set_freq((uint64_t)freq * (uint64_t)100ULL, SI5351_CLK0);
        
        delay(stepDelayMs);
    }

    si5351.output_enable(SI5351_CLK0, 1);
    
    delay(1000);
  }
}

void loop() {}















