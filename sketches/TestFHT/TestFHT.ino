
#include "PAL.h"

//#define LOG_OUT 1 // use the log output function
#define LIN_OUT 1
//#define LIN_OUT8 1
#define FHT_N 256 // set to 256 point fht

#include "FHT.h"


void setup()
{
    @fix@Serial.begin(9600);

    // Set up ADC
    //TIMSK0 = 0; // turn off timer0 for lower jitter
    ADCSRA = 0xe5; // set the adc to free running mode
    ADMUX = 0x40; // use adc0
    DIDR0 = 0x01; // turn off the digital input for adc0

    while (1)
    {
        //cli();
        uint32_t timeStart = PAL.Micros();
        
        // Collect samples
        for (uint16_t i = 0; i < FHT_N; ++i)
        {
            while(!(ADCSRA & 0x10)); // wait for adc to be ready
            ADCSRA = 0xf5; // restart adc
            byte m = ADCL; // fetch adc data
            byte j = ADCH;
            int k = (j << 8) | m; // form into an int
            k -= 0x0200; // form into a signed int
            k <<= 6; // form into a 16b signed int
            fht_input[i] = k; // put real data into bins
        }

        uint32_t timeEndCollect = PAL.Micros();
        
        //sei();
        
        // Process samples
        fht_window(); // window the data for better frequency response
        fht_reorder(); // reorder the data before doing the fht
        fht_run(); // process the data in the fht
        //fht_mag_log(); // take the output of the fht
        fht_mag_lin(); // take the output of the fht
        //fht_mag_lin8();

        uint32_t timeEndProcess = PAL.Micros();

        // Send data
        @fix@Serial.print("FHT_N: "); @fix@Serial.println(FHT_N);
        @fix@Serial.print("Collect Time: ");  @fix@Serial.println(timeEndCollect - timeStart);
        @fix@Serial.print("Process Time: ");  @fix@Serial.println(timeEndProcess - timeEndCollect);
        @fix@Serial.print("Total   Time: ");  @fix@Serial.println(timeEndProcess - timeStart);

        uint32_t perSampleTimeUs = (timeEndCollect - timeStart) / FHT_N;
        uint32_t samplesPerSec   = 1000000 / perSampleTimeUs;
        
        @fix@Serial.print("Per-Sample Time (us): "); @fix@Serial.println(perSampleTimeUs);
        @fix@Serial.print("Samples per-sec: "); @fix@Serial.println(samplesPerSec);
        for (uint16_t i = 0; i < FHT_N/2; ++i)
        {
            @fix@Serial.print(i);
            @fix@Serial.print(": ");
            //@fix@Serial.println(fht_log_out[i]);
            @fix@Serial.println(fht_lin_out[i]);
            //@fix@Serial.println(fht_lin_out8[i]);
        }
        @fix@Serial.println();
        
    }

}

void loop() {}




