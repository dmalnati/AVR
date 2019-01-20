#ifndef __SENSOR_IR_REMOTE_H__
#define __SENSOR_IR_REMOTE_H__


#include "InterruptEventHandler.h"


class SensorIR_Remote
{
public:

    enum class Button : uint32_t
    {
        ZERO       = 0,
        
        POWER      = 16757325,  // 0b00000000111111111011001001001101
        SOURCE     = 16722645,  // 0b00000000111111110010101011010101
        MUTE       = 16738455,  // 0b00000000111111110110100010010111
        
        RECORD     = 16724685,  // 0b00000000111111110011001011001101
        CH_PLUS    = 16752735,  // 0b00000000111111111010000001011111
        TIME_SHIFT = 16724175,  // 0b00000000111111110011000011001111
        
        VOL_MINUS  = 16732335,  // 0b00000000111111110101000010101111
        FULLSCREEN = 16712445,  // 0b00000000111111110000001011111101
        VOL_PLUS   = 16742535,  // 0b00000000111111110111100010000111
        
        NUM_0      = 16730295,  // 0b00000000111111110100100010110111
        CH_MINUS   = 16728255,  // 0b00000000111111110100000010111111
        RECALL     = 16726215,  // 0b00000000111111110011100011000111
        
        NUM_1      = 16748655,  // 0b00000000111111111001000001101111
        NUM_2      = 16758855,  // 0b00000000111111111011100001000111
        NUM_3      = 16775175,  // 0b00000000111111111111100000000111
        
        NUM_4      = 16756815,  // 0b00000000111111111011000001001111
        NUM_5      = 16750695,  // 0b00000000111111111001100001100111
        NUM_6      = 16767015,  // 0b00000000111111111101100000100111
        
        NUM_7      = 16746615,  // 0b00000000111111111000100001110111
        NUM_8      = 16754775,  // 0b00000000111111111010100001010111
        NUM_9      = 16771095,  // 0b00000000111111111110100000010111
    };
    
    uint8_t GetButton(uint32_t val, Button &b)
    {
        uint8_t retVal = 1;
        
        switch ((Button)val)
        {
        case Button::ZERO:       { b = Button::ZERO;       break; }
        case Button::POWER:      { b = Button::POWER;      break; }
        case Button::SOURCE:     { b = Button::SOURCE;     break; }
        case Button::MUTE:       { b = Button::MUTE;       break; }
        case Button::RECORD:     { b = Button::RECORD;     break; }
        case Button::CH_PLUS:    { b = Button::CH_PLUS;    break; }
        case Button::TIME_SHIFT: { b = Button::TIME_SHIFT; break; }
        case Button::VOL_MINUS:  { b = Button::VOL_MINUS;  break; }
        case Button::FULLSCREEN: { b = Button::FULLSCREEN; break; }
        case Button::VOL_PLUS:   { b = Button::VOL_PLUS;   break; }
        case Button::NUM_0:      { b = Button::NUM_0;      break; }
        case Button::CH_MINUS:   { b = Button::CH_MINUS;   break; }
        case Button::RECALL:     { b = Button::RECALL;     break; }
        case Button::NUM_1:      { b = Button::NUM_1;      break; }
        case Button::NUM_2:      { b = Button::NUM_2;      break; }
        case Button::NUM_3:      { b = Button::NUM_3;      break; }
        case Button::NUM_4:      { b = Button::NUM_4;      break; }
        case Button::NUM_5:      { b = Button::NUM_5;      break; }
        case Button::NUM_6:      { b = Button::NUM_6;      break; }
        case Button::NUM_7:      { b = Button::NUM_7;      break; }
        case Button::NUM_8:      { b = Button::NUM_8;      break; }
        case Button::NUM_9:      { b = Button::NUM_9;      break; }
        default:                 { retVal = 0;             break; }
        }
        
        return retVal;
    }
    
public:
    SensorIR_Remote(uint8_t pin)
    : pin_(pin)
    , ied_(pin, LEVEL_FALLING)
    {
        // Nothing to do
    }
    
    void SetCallback(function<void(Button)> cbFn)
    {
        cbFn_ = cbFn;
    }
    
    void Start()
    {
        ied_.SetCallback([this](uint8_t){
            OnFirstLow();
        });
        
        ied_.RegisterForInterruptEvent();
    }
    
    void Stop()
    {
        ied_.DeRegisterForInterruptEvent();
    }


private:

    void OnFirstLow()
    {
        // Decode the bitset
        uint32_t bitBuf = 0;
        
        // Should be in the low period right now.  That should be around 10ms.
        static const uint32_t TIMEOUT_FIRST_LOW_US  = 10000L;
        static const uint32_t TIMEOUT_FIRST_HIGH_US =  5000L;
        
        // Check we're currently low and wait to go high
        if (PAL.DigitalWatch(pin_, LOW, TIMEOUT_FIRST_LOW_US))
        {
            // Check we're currently high and wait to go low
            uint32_t durationFirstHighUs;
            if (PAL.DigitalWatch(pin_, HIGH, TIMEOUT_FIRST_HIGH_US, &durationFirstHighUs))
            {
                // At this point it might be a "repeat" message, identified by
                // the fact that the initial HIGH is ~2.2ms, as opposed to the
                // ~4.4ms when there is a fully qualified code.
                //
                // We choose to convey this as a zero value, as that is all
                // the bits we received to modify the initial value.
                
                if (durationFirstHighUs > 2000 && durationFirstHighUs < 4000)
                {
                    cbFn_(Button::ZERO);
                }
                else
                {
                    // Not a "Repeat" message, read in the 32 bits
                    
                    static const uint32_t TIMEOUT_BIT_LOW_US  = 1000;
                    static const uint32_t TIMEOUT_BIT_HIGH_US = 2000;
                    
                    uint32_t durationBitHighUs = 0;
                    
                    uint8_t cont = 1;
                    uint8_t bitCount = 0;
                    uint8_t transitionMeasured =
                        PAL.DigitalChangeWatch(pin_,
                                               HIGH,
                                               TIMEOUT_BIT_LOW_US,
                                               TIMEOUT_BIT_HIGH_US,
                                               &durationBitHighUs);
                                               
                    while (transitionMeasured && cont)
                    {
                        // Interpret the bit value duration.
                        // 1 is ~= 1.625ms
                        // 0 is ~= 0.522ms
                        
                        uint8_t bitVal;
                        if (durationBitHighUs < 1000)
                        {
                            bitVal = 0;
                        }
                        else
                        {
                            bitVal = 1;
                        }
                        
                        // Record bit value in buffer
                        bitBuf = (bitBuf << 1) | bitVal;
                        
                        // Keep track of bits taken in
                        ++bitCount;

                        // Take next measurement if any bits remain
                        if (bitCount < 32)
                        {
                            transitionMeasured =
                                PAL.DigitalChangeWatch(pin_,
                                                       HIGH,
                                                       TIMEOUT_BIT_LOW_US,
                                                       TIMEOUT_BIT_HIGH_US,
                                                       &durationBitHighUs);
                        }
                        else
                        {
                            // Bits captured
                            cont = 0;
                            
                            // Fire callback
                            Button b;
                            if (GetButton(bitBuf, b))
                            {
                                cbFn_(b);
                            }
                        }
                    }
                }
            }
        }
    }
    
    Pin pin_;
    function<void(Button)> cbFn_;
    InterruptEventHandlerDelegate ied_;
};


























#endif  // __SENSOR_IR_REMOTE_H__








