#ifndef __WSPR_MESSAGE_TRANSMITTER_H__
#define __WSPR_MESSAGE_TRANSMITTER_H__


#include "PAL.h"
#include "Function.h"
#include "si5351.wspr.h"
#include "WSPRMessage.h"
#include "WSPREncoder.h"

class WSPRMessageTransmitter
{
public:

    static const uint32_t WSPR_DEFAULT_DIAL_FREQ               = 14095600UL;  // 20 meter band
    static const uint8_t  WSPR_DEFAULT_CHANNEL                 = 7;          // center channel, 34 total
    static const uint16_t WSPR_OFFSET_FROM_DIAL_TO_USABLE_HZ   = 1400;        // leads to 200 Hz area where transmissions are valid
    static const uint16_t WSPR_CHANNEL_BANDWIDTH_HUNDREDTHS_HZ = 586;         // 5.8592 Hz
    
    static const uint8_t  WSPR_SYMBOL_COUNT               = 162;
    static const uint16_t WSPR_TONE_SPACING_HUNDREDTHS_HZ = 146;  // 1.4648 Hz
    static const uint16_t WSPR_DELAY_MS                   = 683;
    
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
    
    uint32_t GetCalculatedFreqHundredths()
    {
        return (WSPR_DEFAULT_DIAL_FREQ * 100UL) +
               (WSPR_OFFSET_FROM_DIAL_TO_USABLE_HZ * 100UL) +
               (WSPR_DEFAULT_CHANNEL * WSPR_CHANNEL_BANDWIDTH_HUNDREDTHS_HZ);
    }
    
    void SetFreqHundredths(uint32_t freq)
    {
        radio_.set_freq(freq, SI5351_CLK0);
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
        RadioOff();
        
        // Apply calibration values
        // - param 1 - assuming 10pF load capacitence
            // notably didn't see difference at 10MHz testing between 8 and 10pF
        // - param 2 - assuming 25MHz crystal, 0 means this
        // - param 3 - tunable calibration value
        radio_.init(SI5351_CRYSTAL_LOAD_10PF,
                    0,
                    calibration_.crystalCorrectionFactor);
        
        // Tune to default freq
        SetFreqHundredths(GetCalculatedFreqHundredths());

        // Configure to drive at max power
        radio_.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
        
        // Enable the clock
        radio_.set_clock_pwr(SI5351_CLK0, 1);
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
        uint32_t msBitDuration = WSPR_DELAY_MS - calibration_.systemClockOffsetMs;
        
        // Clock out the bits
        for(uint8_t i = 0; i < WSPR_SYMBOL_COUNT; i++)
        {
            uint32_t timeStart = PAL.Millis();
            
            // Change bit
            uint32_t freqInHundrethds = 
                GetCalculatedFreqHundredths() + 
                (wsprEncoder_.GetToneValForSymbol(i) * WSPR_TONE_SPACING_HUNDREDTHS_HZ);
            
            SetFreqHundredths(freqInHundrethds);

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
        // Disable the clock and cut power for all 3 channels
        
        radio_.set_clock_pwr(SI5351_CLK0, 0);
        radio_.output_enable(SI5351_CLK0, 0);
        
        radio_.output_enable(SI5351_CLK1, 0);
        radio_.set_clock_pwr(SI5351_CLK1, 0);
        
        radio_.output_enable(SI5351_CLK2, 0);
        radio_.set_clock_pwr(SI5351_CLK2, 0);
    }
    
    
private:

    Si5351 radio_;
    
    WSPREncoder wsprEncoder_;
    
    Calibration calibration_;
    
    function<void()> fnOnBitChange_;

};





#endif  // __WSPR_MESSAGE_TRANSMITTER_H__








