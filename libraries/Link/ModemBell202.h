#ifndef __MODEM_BELL_202_H__
#define __MODEM_BELL_202_H__


#include "PAL.h"
#include "Timer1.h"
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
    
    static const uint16_t BELL_202_FREQ_SPACE = 2200;
    static const uint16_t BELL_202_FREQ_MARK  = 1200;
    
    static const uint8_t BIT_STUFF_AFTER_COUNT = 5;
    
    static const uint8_t COMMAND_QUEUE_CAPACITY = 8;
    
    constexpr static const double AVR_CLOCK_SCALING_FACTOR = 1.0085;
    
    using SignalDACType = SignalDAC<SignalSourceSineWave>;
    
    

    // Build the messages from the main thread to the ISR thread which will
    // indicate which frequency to change to at each bit boundary.
    enum class CommandType : uint8_t
    {
        NOP = 0,
        CHANGE_FREQUENCY
    };
        
    struct Command
    {
        CommandType cmdType;
        
        union
        {
            SignalDACType::FrequencyConfig *dacCfg;
        };
    };

    using CommandQueue = Queue<Command, COMMAND_QUEUE_CAPACITY>;
    
    
public:
    ModemBell202()
    : dacCfgList_{&dacCfg1200_, &dacCfg2200_}
    , timerChannelA_(timer_.GetTimerChannelA())
    , timerChannelOvf_(timer_.GetTimerOverflowHandler())
    , timerTopValue_(CalculateTimerTopValue())
    {
        Reset();
        
        // Get configuration for both frequencies to be used
        dac_.GetFrequencyConfig(BELL_202_FREQ_SPACE, &dacCfg2200_);
        dac_.GetFrequencyConfig(BELL_202_FREQ_MARK,  &dacCfg1200_);
    }
    
    ~ModemBell202() {}
    
    void Start()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            Reset();
            
            // Doesn't matter which frequency starts, it's NRZI, so
            // it's really the transitions which matter
            dac_.SetInitialFrequency(dacCfgList_[dacCfgListIdx_]);
            
            // Set up timer to count fast (high-res) and high (16-bit) so that
            // we can specify a long duration between match events
            timer_.SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
            timer_.SetTimerMode(Timer1::TimerMode::FAST_PWM_TOP_OCRNA);
            timer_.SetTimerValue(0);
            
            // Set up handler for baud-rate callback by having the timer wrap
            // every 833.3333...us for 1200 baud
            timerChannelA_->SetValue(timerTopValue_);
            
            // Set up handler for when the wrap (actually equality) occurs.
            // We're looking for the main-thread code to have pushed commands
            // onto our queue, which represent bit transitions.
            // These bit transitions will cause us to change the DAC output
            // frequency.
            timerChannelA_->SetInterruptHandler([this](){
                Command cmd;
                
                if (cmdQueue_.Pop(cmd))
                {
                    if (cmd.cmdType == CommandType::CHANGE_FREQUENCY)
                    {
                        dac_.ChangeFrequencyNonAtomic(cmd.dacCfg);
                    }
                }
            });
            timerChannelA_->RegisterForInterrupt();
            
            
            // Debug only -- Check period of bit transition timeout
            timerChannelA_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::SPECIAL_TOP_VALUE);
            
            
            // Shut off Timer0, it interferes
            //PAL.PowerDownTimer0();
            
            
            // Get DAC going
            dac_.Start();
            
            // Get our timer going.
            // Expected operation is that immediately after Start the Send
            // function is called to start pushing bits into the command queue.
            //timer_.StartTimer();
        }
    }
    
    void Send(uint8_t *buf, uint8_t  bufLen, uint8_t  bitStuff = 1)
    {
        // Bit counting re-sets every Send
        bitStuffCount_ = 0;
        
        for (uint8_t i = 0; i < bufLen; ++i)
        {
            SendByte(buf[i], bitStuff);
        }
    }
    
    void Stop()
    {
        // Wait for completion!
        //
        // Can very easily out-pace the DAC, so need to wait for it to finish.
        Command cmd;
        cmd.cmdType = CommandType::NOP;
        
        cmdQueue_.PushAtomic(cmd);
        
        while (cmdQueue_.Size()) {}

        
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Stop the DAC
            dac_.Stop();
            
            // Don't let any potentially queued interrupts fire
            timerChannelOvf_->DeRegisterForInterrupt();
            
            // Stop the timer
            timer_.StopTimer();
            
            // Turn on Timer0 again
            //PAL.PowerUpTimer0();
        }
    }

    
private:

    void Reset()
    {
        dacCfgListIdx_ = 0;
        bitStuffCount_ = 0;
        
        cmdQueue_.Clear();
    }

    void SendByte(uint8_t b, uint8_t bitStuff)
    {
        uint8_t bTmp = b;
        
        for (uint8_t i = 0; i < 8; ++i)
        {
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
        
        // Push a command onto the queue for the next baud interval
        Command cmd;
        
        cmd.cmdType = CommandType::CHANGE_FREQUENCY;
        cmd.dacCfg  = dacCfgList_[dacCfgListIdx_];
        
        cmdQueue_.PushAtomic(cmd);
    }
    
    
    static uint16_t CalculateTimerTopValue()
    {
        // A few constants for our 8MHz, no-prescaler, 16-bit timer
        double US_PER_TICK = 0.125;

        // Start with the actual period you want
        double periodLogicalUs = 1000.0 / BAUD * 1000.0;

        // Now account for AVR clock running at not quite wall-clock speed
        double periodUs = periodLogicalUs * AVR_CLOCK_SCALING_FACTOR;

        // Convert the duration in us into ticks of the timer
        double ticksPerPeriod = periodUs / US_PER_TICK;

        uint16_t top = ticksPerPeriod - 1;
        
        return top;
    }
    

    
    
    uint8_t bitStuffCount_;    

    SignalDACType                    dac_;
    SignalDACType::FrequencyConfig   dacCfg1200_;
    SignalDACType::FrequencyConfig   dacCfg2200_;
    SignalDACType::FrequencyConfig  *dacCfgList_[2];
    uint8_t                          dacCfgListIdx_;
    
    Timer1          timer_;
    TimerChannel   *timerChannelA_;
    TimerInterrupt *timerChannelOvf_;

    uint16_t  timerTopValue_;
    
    CommandQueue  cmdQueue_;
};






#endif  // __MODEM_BELL_202_H__




















