#include <si5351.h>
#include "WSPREncoder.h"
#include "UtlStreamBlob.h"



#define BUTTON                  12
#define LED_PIN                 13


Si5351 si5351;
WSPREncoder wsprEncoder;


void Transmit()
{

#define WSPR_DEFAULT_FREQ       14097200UL
#define WSPR_TONE_SPACING       146           // ~1.46 Hz
#define WSPR_DELAY              683          // Delay value for WSPR
#define WSPR_SYMBOL_COUNT                   162


    
    uint32_t freq = WSPR_DEFAULT_FREQ;
    uint8_t  symbol_count = WSPR_SYMBOL_COUNT; // From the library defines
    uint16_t tone_spacing = WSPR_TONE_SPACING;
    uint16_t tone_delay = WSPR_DELAY;
    
  si5351.output_enable(SI5351_CLK0, 1);

  for(uint8_t i = 0; i < symbol_count; i++)
  {
      //si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), SI5351_CLK0);
      si5351.set_freq((freq * 100) + (wsprEncoder.msg[i] * tone_spacing), SI5351_CLK0);
      //delay(tone_delay);

      static const int16_t offset = -8;
      delay(tone_delay + offset);
  }

  si5351.output_enable(SI5351_CLK0, 0);
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");
    
  // Initialize the Si5351
  // Change the 2nd parameter in init if using a ref osc other
  // than 25 MHz
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);

  // Use a button connected to pin 12 as a transmit trigger
  pinMode(BUTTON, INPUT_PULLUP);

  // Set CLK0 output
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // Set for max power if desired
  si5351.output_enable(SI5351_CLK0, 0); // Disable the clock initially

  // Encode the message in the transmit buffer

    //char message[] = "N0CALL AA00";
    char call[] = "KD2KDD";
    char loc[] = "AA00";
    uint8_t dbm = 27;

  wsprEncoder.genmsg(call, loc, dbm);

  Serial.println("Buffer:");
  StreamBlob(Serial, wsprEncoder.msg, 162, 1, 1);
  Serial.println();
}

void loop()
{
  // Debounce the button and trigger TX on push
  if(digitalRead(BUTTON) == LOW)
  {
    delay(50);   // delay to debounce
    if (digitalRead(BUTTON) == LOW)
    {
        Serial.println("Encoding");
        
      Transmit();
      
      delay(50); //delay to avoid extra triggers
    }
  }
}


















