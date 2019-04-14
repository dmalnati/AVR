#ifndef __WSPR_MESSAGE_TRANSMITTER_H__
#define __WSPR_MESSAGE_TRANSMITTER_H__


#include "PAL.h"
#include "Function.h"
#include "si5351.wspr.h"
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
        int32_t crystalCorrectionFactor = 0;
        int32_t systemClockOffsetMs     = 0;
    };

    void SetCalibration(Calibration calibration)
    {
        calibration_ = calibration;
    }
    
    void SetCallbackOnBitChange(function<void()> fnOnBitChange)
    {
        fnOnBitChange_ = fnOnBitChange;
    }
    
    uint8_t Test(WSPRMessage *msg)
    {
        uint8_t retVal = 0;
        
        // Get access to data in message
        const char *callsign = NULL;
        const char *grid     = NULL;
        uint8_t     powerDbm = 0;
        
        msg->GetData(callsign, grid, powerDbm);
        
        // Test
        uint8_t testOnly = 1;
        retVal = wsprEncoder_.Encode(callsign, grid, powerDbm, testOnly);
        
        return retVal;
    }

    void RadioOn()
    {
        // Apply calibration values
        // - param 1 - assuming 10pF load capacitence
            // notably didn't see difference at 10MHz testing between 8 and 10pF
        // - param 2 - assuming 25MHz crystal, 0 means this
        // - param 3 - tunable calibration value
        radio_.init(SI5351_CRYSTAL_LOAD_10PF,
                    0,
                    calibration_.crystalCorrectionFactor);
        
        // Tune to default freq
        radio_.set_freq(WSPR_DEFAULT_FREQ * 100, SI5351_CLK0);

        // Configure to drive at max power
        radio_.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
        
        // Enable the clock
        radio_.output_enable(SI5351_CLK0, 1);
    }
    
    uint8_t Send(WSPRMessage *msg)
    {
        uint8_t retVal = 0;
        
        // Get access to data in message
        const char *callsign = NULL;
        const char *grid     = NULL;
        uint8_t     powerDbm = 0;
        
        msg->GetData(callsign, grid, powerDbm);
        
        
        // Encode
        retVal = wsprEncoder_.Encode(callsign, grid, powerDbm);
        
        
        // Allow calibration to compensate for system clock not being accurate
        // enough to hit precise bit duration
        uint32_t msBitDuration = WSPR_DELAY_MS + calibration_.systemClockOffsetMs;
        
        
        // Clock out the bits
        for(uint8_t i = 0; i < WSPR_SYMBOL_COUNT; i++)
        {
            uint32_t timeStart = PAL.Millis();
            
            // Change bit
            uint32_t freqInHundrethds = 
                (WSPR_DEFAULT_FREQ * 100) + 
                (wsprEncoder_.GetToneValForSymbol(i) * WSPR_TONE_SPACING);
            
            radio_.set_freq(freqInHundrethds, SI5351_CLK0);

            // Allow calling code to do something here
            fnOnBitChange_();
            
            // Calculate time of next bit transition from where we are now
            uint32_t timeEnd          = PAL.Millis();
            uint32_t msSleepRemaining = (msBitDuration - (timeEnd - timeStart));
            
            PAL.Delay(msSleepRemaining);
        }
        
        return retVal;
    }

    void RadioOff()
    {
        // Disable the clock
        radio_.output_enable(SI5351_CLK0, 0);
    }
    
    
private:

    Si5351 radio_;
    
    WSPREncoder wsprEncoder_;
    
    Calibration calibration_;
    
    function<void()> fnOnBitChange_;
};





#endif  // __WSPR_MESSAGE_TRANSMITTER_H__








