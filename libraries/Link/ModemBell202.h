#ifndef __MODEM_BELL_202_H__
#define __MODEM_BELL_202_H__


#include "PAL.h"
#include "Container.h"
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
    
    using SignalDACType = SignalDAC<SignalSourceSineWave>;
    
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
        
        // Set up command queue
        dac_.SetCommandQueue(&dacCmdQueue_);
        
        // Begin
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
        
        
        // Wait for completion!
        //
        // Can very easily out-pace the DAC, so need to wait for it to finish.
        // Possibly this could be done in Stop(), but why, let's call Send()
        // completely synchronous.
        //
        /*
        SignalDACType::Command cmd;
        
        cmd.type         = SignalDACType::CommandType::NOP;
        cmd.cfgFrequency = dacCfgList_[dacCfgListIdx_];
        
        dacCmdQueue_.PushAtomic(cmd);
        while (dacCmdQueue_.Size()) {}
        */
        
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
        
        dacCmdQueue_.Clear();
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

        //dac_.ChangeFrequency(dacCfgList_[dacCfgListIdx_]);
        
        // Push a command onto the queue for the DAC to consult
        SignalDACType::Command cmd;
        
        cmd.cmdType      = SignalDACType::CommandType::CHANGE_FREQUENCY;
        cmd.cfgFrequency = dacCfgList_[dacCfgListIdx_];
        
        dacCmdQueue_.PushAtomic(cmd);
        
        PAL.DigitalWrite(pinDebugModem_, LOW);
    }
    
    Pin pinDebugModem_;
    
    uint8_t bitStuffCount_;    

    SignalSourceSineWave  sineWave_;
    
    SignalDACType                    dac_;
    SignalDACType::FrequencyConfig   dacCfg1200_;
    SignalDACType::FrequencyConfig   dacCfg2200_;
    SignalDACType::FrequencyConfig  *dacCfgList_[2];
    uint8_t                          dacCfgListIdx_;
    
    SignalDACType::CommandQueue      dacCmdQueue_;
};






#endif  // __MODEM_BELL_202_H__




















