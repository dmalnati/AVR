#ifndef __WSPR_MESSAGE_TRANSMITTER_H__
#define __WSPR_MESSAGE_TRANSMITTER_H__


#include "PAL.h"
#include "Function.h"
#include "si5351.h"
#include "WSPRMessage.h"
#include "WSPREncoder.h"


class WSPRMessageTransmitter
{
    static const uint32_t WSPR_DEFAULT_FREQ = 14097200UL;
    static const uint8_t  WSPR_SYMBOL_COUNT = 162;
    static const uint16_t WSPR_TONE_SPACING = 146;  // ~1.46 Hz
    static const uint16_t WSPR_DELAY_MS     = 683;
    
public:

    struct Calibration
    {
        enum CrystalCapacitance
        {
            CAP_8PF,
            CAP_10PF,
        };
        
        CrystalCapacitance crystalCap = CrystalCapacitance::CAP_8PF;
        
        int32_t clkGenSkew = 0;
        
        int32_t timerDelayOffset = 0;
    };

    void SetCalibration(Calibration calibration)
    {
        calibration_ = calibration;
    }
    
    void SetCallbackRadioOn(function<void()> fnOnRadioOn)
    {
        fnOnRadioOn_ = fnOnRadioOn;
    }
    
    void SetCallbackRadioOff(function<void()> fnOnRadioOff)
    {
        fnOnRadioOff_ = fnOnRadioOff;
    }

    void RadioOn()
    {
        // Fire callback
        fnOnRadioOn_();
        
        // Apply calibration values
        radio_.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
        
        // Configure to drive at max power
        radio_.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
        
        // Enable the clock
        radio_.output_enable(SI5351_CLK0, 1);
    }
    
    void Send(WSPRMessage *msg)
    {
        // Get access to data in message
        const char *callsign = NULL;
        const char *grid     = NULL;
        uint8_t     powerDbm = 0;
        
        msg->GetData(callsign, grid, powerDbm);
        
        
        // Encode
        wsprEncoder_.Encode(callsign, grid, powerDbm);
        
        
        // Clock out the bits
        for(uint8_t i = 0; i < WSPR_SYMBOL_COUNT; i++)
        {
            uint32_t freqInHundrethds = 
                (WSPR_DEFAULT_FREQ * 100) + 
                (wsprEncoder_.GetToneValForSymbol(i) * WSPR_TONE_SPACING);
            
            radio_.set_freq(freqInHundrethds, SI5351_CLK0);

            int16_t offset = -8;
            PAL.Delay(WSPR_DELAY_MS + offset);
        }
    }

    void RadioOff()
    {
        // Disable the clock
        radio_.output_enable(SI5351_CLK0, 0);
        
        // Fire callback
        fnOnRadioOff_();
    }
    
    
private:

    Si5351 radio_;
    
    WSPREncoder wsprEncoder_;
    
    Calibration calibration_;
    
    function<void()> fnOnRadioOn_;
    function<void()> fnOnRadioOff_;
};





#endif  // __WSPR_MESSAGE_TRANSMITTER_H__








