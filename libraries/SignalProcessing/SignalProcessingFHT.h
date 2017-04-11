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

    struct Measurement
    {
        uint16_t *valList;
        uint16_t  valListLen;
        
        /*
        uint32_t timeDiff;
        double usPerSample;
        uint32_t ratePerSec;
        */
    };
    
    uint8_t GetMeasurement(Pin pin, Measurement *m)
    {
        uint8_t retVal = 0;
        
        if (m)
        {
            retVal = 1;
            
            // Get current prescaler, so it can be restored later
            PlatformAbstractionLayer::ADCPrescaler adcPrescalerCurrent = PAL.GetADCPrescaler();
            
            // Set prescaler lower so operations complete more quickly.
            // Wouldn't do this except that's how I see the example code doing
            // it so why not.
            // The spec says stay within 50-200kHz, but at 8MHz clock, and a
            // prescaler of 32, you get 8,000,000 / 32 = 250kHz, so too quick...
            PAL.SetADCPrescaler(PlatformAbstractionLayer::ADCPrescaler::DIV_BY_32);

            // Begin batch acquire of analog samples
            PAL.AnalogReadBatchBegin();
            
            //uint32_t timeStart = PAL.Micros();
            for (uint16_t i = 0; i < FHT_N; ++i)
            {
                int16_t k = (int16_t)PAL.AnalogRead(pin);
                
                k -= 0x0200; // form into a signed int
                k <<= 6; // form into a 16b signed int
                fht_input[i] = k; // put real data into bins
            }
            //uint32_t timeEnd = PAL.Micros();
            
            // End batch collection
            PAL.AnalogReadBatchEnd();
            
            // Restore ADC prescaler
            PAL.SetADCPrescaler(adcPrescalerCurrent);

            // Debug
            /*
            m->timeDiff = timeEnd - timeStart;
            m->usPerSample = (double)m->timeDiff / FHT_N;
            m->ratePerSec = 1000000.0 / m->usPerSample;
            */

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