#ifndef __SIGNAL_PROCESSING_FHT_H__
#define __SIGNAL_PROCESSING_FHT_H__


#include "PAL.h"

// Define parameters for library, default for now, user has no choice
#define LIN_OUT   1
#define FHT_N   256

#include "FHT.h"



class SignalProcessingFHT
{
public:

    void Init()
    {
        ADCSRA = 0xe5; // set the adc to free running mode
        ADMUX = 0x40; // use adc0
        DIDR0 = 0x01; // turn off the digital input for adc0
    }
    
    struct Measurement
    {
        uint16_t *valList;
        uint16_t  valListLen;
    };
    
    uint8_t GetMeasurement(Measurement *m)
    {
        uint8_t retVal = 0;
        
        if (m)
        {
            retVal = 1;
            
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

            // Process samples
            fht_window(); // window the data for better frequency response
            fht_reorder(); // reorder the data before doing the fht
            fht_run(); // process the data in the fht
            fht_mag_lin(); // take the output of the fht
            
            m->valList    = fht_lin_out;
            m->valListLen = FHT_N;
        }
        
        return retVal;
    }

private:
};


#endif  // __SIGNAL_PROCESSING_FHT_H__