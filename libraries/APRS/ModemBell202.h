#ifndef __MODEM_BELL_202_H__
#define __MODEM_BELL_202_H__


#include "PAL.h"
#include "Timer1.h"
#include "Container.h"
#include "ModemAnalog.h"


// Physical layer handler

// Will
// - send LSB first for a given byte
// - encoding - NRZI
// - Bit stuffing after 5th consecutive 1

template <typename ModemAnalogType,
          uint8_t  preEmphMark,
          uint8_t  preEmphSpace>
class ModemBell202
{
public:
    using ModemAnalogTypeName = ModemAnalogType;

private:
    static const uint16_t BAUD = 1200;
    
    static const uint16_t BELL_202_FREQ_SPACE = 2200;
    static const uint16_t BELL_202_FREQ_MARK  = 1200;
    
    static const uint8_t BIT_STUFF_AFTER_COUNT = 5;
    
    static const uint8_t COMMAND_QUEUE_CAPACITY = 8;
    

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
        
        ModemAnalogFrequencyConfig fc;
    };

    using CommandQueue = Queue<Command, COMMAND_QUEUE_CAPACITY>;
    
    
public:
    ModemBell202()
    : timerChannelA_(timer_.GetTimerChannelA())
    , timerChannelOvf_(timer_.GetTimerOverflowHandler())
    , timerTopValue_(CalculateTimerTopValue())
    {
        Reset();
    }
    
    ~ModemBell202() {}
    
    void Init()
    {
        ma_.Init();
        
        fcList_[0] = ma_.GetFrequencyConfig(BELL_202_FREQ_MARK,  preEmphMark);
        fcList_[1] = ma_.GetFrequencyConfig(BELL_202_FREQ_SPACE, preEmphSpace);
    }
    
    void Start()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            Reset();
            
            // Doesn't matter which frequency starts, it's NRZI, so
            // it's really the transitions which matter
            ma_.SetFrequencyByConfig(&fcList_[freqListIdx_]);
            
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
            // These bit transitions will cause us to change the output
            // frequency.
            timerChannelA_->SetInterruptHandlerRaw(OnInterrupt);
            timerChannelA_->RegisterForInterrupt();
            
            // Debug only -- Check period of bit transition timeout
            //timerChannelA_->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::SPECIAL_TOP_VALUE);
            
            // Get output signal going
            ma_.Start();
            
            // Get our timer going.
            // Expected operation is that immediately after Start the Send
            // function is called to start pushing bits into the command queue.
            timer_.StartTimer();
        }
    }
    
    static void OnInterrupt()
    {
        Command cmd;
        
        if (cmdQueue_.Pop(cmd))
        {
            if (cmd.cmdType == CommandType::CHANGE_FREQUENCY)
            {
                ma_.SetFrequencyByConfig(&cmd.fc);
            }
        }
    }
    
    void Send(uint8_t *buf, uint8_t  bufLen, uint8_t  bitStuff = 1)
    {
        // Bit counting re-sets every Send
        consecutiveOnes_ = 0;
        
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
            ma_.Stop();
            
            // Don't let any potentially queued interrupts fire
            timerChannelOvf_->DeRegisterForInterrupt();
            
            // Stop the timer
            timer_.StopTimer();
        }
    }

    
private:

    void Reset()
    {
        freqListIdx_ = 0;
        consecutiveOnes_ = 0;
        
        cmdQueue_.Clear();
    }

    void SendByte(uint8_t b, uint8_t bitStuff)
    {
        uint8_t bTmp = b;
        
        for (uint8_t i = 0; i < 8; ++i)
        {
            // Get next bit -- LSB first
            uint8_t bitVal = bTmp & 0x01;
            
            // Set up byte for next iteration
            bTmp >>= 1;
            
            // Send this bit
            SendBit(bitVal);
            
            
            // Consider whether a bit needs to get stuffed
            
            // Is bit stuffing even enabled?
            if (bitStuff)
            {
                // Is this a 1?  We're watching for consecutive 1s.
                if (bitVal)
                {
                    ++consecutiveOnes_;
                    
                    if (consecutiveOnes_ == BIT_STUFF_AFTER_COUNT)
                    {
                        SendBit(0);
                        
                        // reset
                        consecutiveOnes_ = 0;
                    }
                }
                else
                {
                    consecutiveOnes_ = 0;
                }
            }
        }
    }
    
    inline void SendBit(uint8_t bitVal)
    {
        // NRZI -- transition on 0s
        
        if (!bitVal)
        {
            // do transition
            freqListIdx_ = !freqListIdx_;
            
            // Push a ChangeFrequency command onto the queue for the
            // next baud interval
            Command cmd;
            
            cmd.cmdType = CommandType::CHANGE_FREQUENCY;
            cmd.fc  = fcList_[freqListIdx_];
            
            cmdQueue_.PushAtomic(cmd);
        }
        else
        {
            // no transition
            
            // Push a No-Op command onto the queue.
            // If nothing was pushed, the ISR would ultimately find the next 
            // ChangeFrequency command as the main thread moved through all the
            // bits to send.
            Command cmd;
            cmd.cmdType = CommandType::NOP;
            
            cmdQueue_.PushAtomic(cmd);
        }
    }
    
    
    static uint16_t CalculateTimerTopValue()
    {
        // A few constants for our 8MHz, no-prescaler, 16-bit timer
        double US_PER_TICK = 0.125;

        // Start with the actual period you want
        double periodLogicalUs = 1000.0 / BAUD * 1000.0;

        // Convert the duration in us into ticks of the timer
        double ticksPerPeriod = periodLogicalUs / US_PER_TICK;

        //uint16_t top = ticksPerPeriod - 1;
        // Fixes timing, but why?
        //uint16_t top = ticksPerPeriod + 75;   // worked with DAC
        uint16_t top = ticksPerPeriod - 156;    // worked with PWM
        
        return top;
    }
    

    
    
    uint8_t consecutiveOnes_;    
    
    static ModemAnalogType ma_;

    ModemAnalogFrequencyConfig fcList_[2];
    uint8_t   freqListIdx_;
    
    Timer1          timer_;
    TimerChannel   *timerChannelA_;
    TimerInterrupt *timerChannelOvf_;

    uint16_t  timerTopValue_;
    
    static CommandQueue  cmdQueue_;
};


template <typename ModemAnalogType, uint8_t  preEmphMark, uint8_t  preEmphSpace>
typename ModemBell202<ModemAnalogType,preEmphMark,preEmphSpace>::ModemAnalogTypeName ModemBell202<ModemAnalogType,preEmphMark,preEmphSpace>::ma_;

template <typename ModemAnalogType, uint8_t  preEmphMark, uint8_t  preEmphSpace>
typename ModemBell202<ModemAnalogType,preEmphMark,preEmphSpace>::CommandQueue ModemBell202<ModemAnalogType,preEmphMark,preEmphSpace>::cmdQueue_;


// Define a few easy to use options
using ModemBell202Dac = ModemBell202<ModemAnalogDac, 33, 99>;
using ModemBell202Pwm = ModemBell202<ModemAnalogPwm, 25, 50>;



#endif  // __MODEM_BELL_202_H__




















