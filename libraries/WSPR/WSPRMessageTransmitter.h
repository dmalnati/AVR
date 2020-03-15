#ifndef __WSPR_MESSAGE_TRANSMITTER_H__
#define __WSPR_MESSAGE_TRANSMITTER_H__


#include "PAL.h"
#include "Function.h"
// #include "si5351.h"
//#include "si5351.wspr.h"
#include "si5351.wspr.2.h"
#include "WSPRMessage.h"
#include "WSPREncoder.h"

class WSPRMessageTransmitter
{
public:

    static const uint8_t WSPR_CHANNEL_LOW     = 0;
    static const uint8_t WSPR_CHANNEL_DEFAULT = 16;          // 33 total, 0-32
    static const uint8_t WSPR_CHANNEL_HIGH    = 32;

    static const uint32_t WSPR_DEFAULT_DIAL_FREQ               = 14095600UL;  // 20 meter band
    static const uint16_t WSPR_OFFSET_FROM_DIAL_TO_USABLE_HZ   = 1400;        // leads to 200 Hz area where transmissions are valid
    static const uint16_t WSPR_CHANNEL_BANDWIDTH_HUNDREDTHS_HZ = 586;         // 5.8592 Hz
    
    static const uint8_t  WSPR_SYMBOL_COUNT               = 162;
    static const uint16_t WSPR_TONE_SPACING_HUNDREDTHS_HZ = 146;  // 1.4648 Hz
    static const uint16_t WSPR_DELAY_MS                   = 683;
    
public:

    struct Calibration
    {
        Calibration()
        : Calibration(0, 0)
        {
            // Nothing to do
        }
        
        Calibration(int32_t crystalCorrectionFactorInput, int32_t systemClockOffsetMsInput)
        : crystalCorrectionFactor(crystalCorrectionFactorInput)
        , systemClockOffsetMs(systemClockOffsetMsInput)
        {
            // Nothing to do
        }
        
        int32_t crystalCorrectionFactor = 0;
        int32_t systemClockOffsetMs     = 0;
    };
    
    void SetCalibration(Calibration calibration)
    {
        calibration_ = calibration;
    }
    
    void SetChannel(uint8_t channel)
    {
        channel_ = channel;
    }
    
    uint8_t GetChannel()
    {
        return channel_;
    }
    
    uint32_t GetCalculatedFreqHundredths()
    {
        return (WSPR_DEFAULT_DIAL_FREQ * 100UL) +
               (WSPR_OFFSET_FROM_DIAL_TO_USABLE_HZ * 100UL) +
               ((uint32_t)GetChannel() * WSPR_CHANNEL_BANDWIDTH_HUNDREDTHS_HZ);
    }
    
    void SetFreqHundredths(uint32_t freq)
    {
        radio_.set_freq(freq, SI5351_CLK0);
    }
    
    void SetCallbackOnBitChange(function<void()> fnOnBitChange)
    {
        fnOnBitChange_ = fnOnBitChange;
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

        // First clock used to set the frequency
        radio_.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
        radio_.set_clock_pwr(SI5351_CLK0, 1);
        radio_.output_enable(SI5351_CLK0, 1);

        // Fan out and invert the first clock signal for a
        // 180-degree phase shift on second clock
        radio_.set_clock_fanout(SI5351_FANOUT_MS, 1);
        radio_.set_clock_source(SI5351_CLK1, SI5351_CLK_SRC_MS0);
        radio_.set_clock_invert(SI5351_CLK1, 1);

        radio_.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
        radio_.set_clock_pwr(SI5351_CLK1, 1);
        radio_.output_enable(SI5351_CLK1, 1);
    }
    
    void Send(WSPRMessage *msg)
    {
        // Get access to data in message
        const char *callsign = NULL;
        const char *grid     = NULL;
        uint8_t     powerDbm = 0;

        msg->GetData(callsign, grid, powerDbm);
        
        wsprEncoder_.EncodeType1(callsign, grid, powerDbm);
        
        // Allow calibration to compensate for system clock not being accurate
        // enough to hit precise bit duration
        uint32_t msBitDuration = WSPR_DELAY_MS - calibration_.systemClockOffsetMs;
        
        // Keep state allowing more accurate timing for bit periods
        uint8_t  firstBit     = 1;
        uint32_t timeBitStart = 0;
        
        // Clock out the bits
        for(uint8_t i = 0; i < WSPR_SYMBOL_COUNT; i++)
        {
            // Determine which bit to send next
            uint32_t freqInHundrethds = 
                GetCalculatedFreqHundredths() + 
                (wsprEncoder_.GetToneValForSymbol(i) * WSPR_TONE_SPACING_HUNDREDTHS_HZ);
            
            // We want to send the first bit immediately.
            // We want to send subsequent bits at one bit-duration later
            if (!firstBit)
            {
                // Calculate how long to wait before beginning the next bit
                uint32_t timeNow          = PAL.Millis();
                uint32_t timeDiff         = timeNow - timeBitStart;
                uint32_t msSleepRemaining = (msBitDuration - timeDiff);
                
                PAL.Delay(msSleepRemaining);
            }
            else
            {
                firstBit = 0;
            }
            
            // Keep track of when this bit started transmitting
            timeBitStart = PAL.Millis();
            
            // Actually change the frequency to indicate the bit value
            SetFreqHundredths(freqInHundrethds);
            
            // Allow calling code to do something here
            fnOnBitChange_();
        }
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

    void SetDrive(si5351_drive pwr)
    {
        radio_.drive_strength(SI5351_CLK0, pwr);
        radio_.drive_strength(SI5351_CLK1, pwr);
    }
    void SetDrive2() { SetDrive(SI5351_DRIVE_2MA); }
    void SetDrive4() { SetDrive(SI5351_DRIVE_4MA); }
    void SetDrive6() { SetDrive(SI5351_DRIVE_6MA); }
    void SetDrive8() { SetDrive(SI5351_DRIVE_8MA); }
    
    
private:

    Si5351 radio_;
    
    WSPREncoder wsprEncoder_;
    
    Calibration calibration_;
    
    uint8_t channel_ = WSPR_CHANNEL_DEFAULT;
    
    function<void()> fnOnBitChange_;

};





#endif  // __WSPR_MESSAGE_TRANSMITTER_H__








