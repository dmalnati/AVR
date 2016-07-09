#ifndef __MODEM_BELL_202_H__
#define __MODEM_BELL_202_H__


#include "PAL.h"
#include "SignalSourceSineWave.h"
#include "SignalDAC.h"


// Physical layer handler

// Need to handle being configured to:
// - send LSB or MSB first for a given byte
// - encoding - NRZI
// - Bit stuffing might best be done here...

class ModemBell202
{
    static const uint16_t BAUD = 1200;
    
    static const uint16_t BIT_DURATION_US = 1000.0 / BAUD * 1000.0;
    
    static const uint16_t BELL_202_FREQ_SPACE = 2200;
    static const uint16_t BELL_202_FREQ_MARK  = 1200;
    
    static const uint8_t BIT_STUFF_AFTER_COUNT = 5;
    
public:
    ModemBell202()
    : pinDebugModem_(6)
    , dac_(&sineWave_)
    , dacCfgList_{&dacCfg1200_, &dacCfg2200_}
    {
        Reset();
        
        // Get configuration for both frequencies to be used
        dac_.GetFrequencyConfig(BELL_202_FREQ_SPACE, &dacCfg2200_);
        dac_.GetFrequencyConfig(BELL_202_FREQ_MARK,  &dacCfg1200_);
        
        PAL.PinMode(pinDebugModem_, OUTPUT);
        PAL.DigitalWrite(pinDebugModem_, LOW);
    }
    
    ~ModemBell202() {}
    
    inline void Start()
    {
        Reset();
        
        // Doesn't matter which frequency starts, it's NRZI, so
        // it's really the transitions which matter
        dac_.SetInitialFrequency(dacCfgList_[dacCfgListIdx_]);
        
        dac_.Start();
    }
    
    inline void Send(uint8_t *buf, uint8_t  bufLen, uint8_t  bitStuff = 1)
    {
        // Bit counting re-sets every Send
        bitStuffCount_ = 0;
        
        for (uint8_t i = 0; i < bufLen; ++i)
        {
            SendByte(buf[i], bitStuff);
        }
    }
    
    inline void Stop()
    {
        dac_.Stop();
    }

    
private:

    void Reset()
    {
        dacCfgListIdx_ = 0;
        bitStuffCount_ = 0;
    }

    void SendByte(uint8_t b, uint8_t bitStuff)
    {
        uint8_t bTmp = b;
        
        for (uint8_t i = 0; i < 8; ++i)
        {
            PAL.DigitalWrite(pinDebugModem_, HIGH);
            
            // Get next bit -- assume LSB first
            uint8_t bitVal = bTmp & 0x01;
            
            // Set up byte for next iteration
            bTmp >>= 1;
            
            // Send this bit
            SendBit(bitVal);
            
            // Consider whether a bit needs to get stuffed
            if (bitVal)
            {
                ++bitStuffCount_;
                
                if (bitStuffCount_ == BIT_STUFF_AFTER_COUNT)
                {
                    // stuff if enabled
                    // (intentionally inefficient to try to keep runtime
                    //  approx the same when enabled vs not)
                    if (bitStuff)
                    {
                        SendBit(0);
                    }
                    
                    // reset
                    bitStuffCount_ = 0;
                }
            }
            else
            {
                bitStuffCount_ = 0;
            }
        }
    }
    
    inline void SendBit(uint8_t bitVal)
    {
        // NRZI -- transition on 0s
        
        if (!bitVal)
        {
            // do transition
            dacCfgListIdx_ = !dacCfgListIdx_;
        }
        else
        {
            // no transition
        }
        
        // Unconditionally call this in order to try to keep run time the
        // same whether or not the frequency changes

        dac_.ChangeFrequency(dacCfgList_[dacCfgListIdx_], &wrapCounter_);
        
        PAL.DigitalWrite(pinDebugModem_, LOW);
        
        // 32usec per timer loop, 833.3333..us per bit, that's 26 wraps per bit
        // Play with the number a bit to adjust
        // 24 gets 8 bits in 6.51ms
        // 25 gets 8 bits in 6.75ms
        // really we want 6.66ms (833us * 8)
        // so toggle based on bit value and hope it averages out?
        // maybe pad out with some asm nops?
        register uint8_t wrapCounterMax = 23;
        while (wrapCounter_ < wrapCounterMax) {}
        
        // just waste some time
        for (uint8_t i = 0; i < 5; ++i)
        {
            ++wrapCounter_;
            
            //for (uint8_t j = 0; j < 254; ++j) { ++wrapCounter_; }
        }
        
        // Wait for bit to be transmitted
        // This is taking forever!
        // Hmm, I tink the timer0 callbacks are getting starved out...
        //PAL.DelayMicroseconds(BIT_DURATION_US);
    }
    
    Pin pinDebugModem_;
    
    uint8_t bitStuffCount_;    

    SignalSourceSineWave  sineWave_;
    
    SignalDAC<SignalSourceSineWave>   dac_;
    SignalDACFrequencyConfig          dacCfg1200_;
    SignalDACFrequencyConfig          dacCfg2200_;
    SignalDACFrequencyConfig         *dacCfgList_[2];
    uint8_t                           dacCfgListIdx_;
    
    volatile uint8_t wrapCounter_;
};






#endif  // __MODEM_BELL_202_H__




















