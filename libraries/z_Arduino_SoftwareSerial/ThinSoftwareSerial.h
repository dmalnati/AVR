#ifndef ThinSoftwareSerial_h
#define ThinSoftwareSerial_h

#include <inttypes.h>
#include <Stream.h>

#include "PAL.h"
#include "PCIntEventHandler.h"
#include "Log.h"


class ThinSoftwareSerial
{
public:

    static const uint8_t BUF_SIZE = 32;


public:

    ThinSoftwareSerial(uint8_t receivePin, uint8_t transmitPin) : 
    _runningAtSpeedFactor(1.0),
    _rx_delay_centering(0),
    _rx_delay_intrabit(0),
    _rx_delay_stopbit(0),
    _tx_delay(0),
    pcied_(receivePin, PCIntEventHandler::MODE::MODE_FALLING)
    {
        setTX(PAL.GetArduinoPinFromPhysicalPin(transmitPin));
        setRX(PAL.GetArduinoPinFromPhysicalPin(receivePin));

        pcied_.SetCallback([this](uint8_t){
            handle_interrupt();
        });
    }

    void SetRunAtSpeedFactor(double runningAtSpeedFactor)
    {
        _runningAtSpeedFactor = runningAtSpeedFactor;
    }


    void begin(long speed)
    {
        _rx_delay_centering = _rx_delay_intrabit = _rx_delay_stopbit = _tx_delay = 0;

        // Precalculate the various delays, in number of 4-cycle delays
        uint16_t bit_delay = (F_CPU / speed) / 4;

        // 12 (gcc 4.8.2) or 13 (gcc 4.3.2) cycles from start bit to first bit,
        // 15 (gcc 4.8.2) or 16 (gcc 4.3.2) cycles between bits,
        // 12 (gcc 4.8.2) or 14 (gcc 4.3.2) cycles from last bit to stop bit
        // These are all close enough to just use 15 cycles, since the inter-bit
        // timings are the most critical (deviations stack 8 times)
        _tx_delay = subtract_cap(bit_delay, 15 / 4);

        // There are 23 cycles in each loop iteration (excluding the delay)
        _rx_delay_intrabit = subtract_cap(bit_delay, 23 / 4);

        
        // at 8MHz the bit is sampled 12us after it starts
        // at 16MHz the bit is sampled 6us after it starts
        // at 9600 baud, bit duration is 104us
        //
        // The number below was manually tuned
        _rx_delay_centering = _rx_delay_intrabit - 34;
        

        // There are 37 cycles from the last bit read to the start of
        // stopbit delay and 11 cycles from the delay until the interrupt
        // mask is enabled again (which _must_ happen during the stopbit).
        // This delay aims at 3/4 of a bit time, meaning the end of the
        // delay will be at 1/4th of the stopbit. This allows some extra
        // time for ISR cleanup, which makes 115200 baud at 16Mhz work more
        // reliably
        _rx_delay_stopbit = subtract_cap(bit_delay * 3 / 4, (37 + 11) / 4);

        tunedDelay(_tx_delay); // if we were low this establishes the end

        // Adjust timing in accordance with clock drift.
        //
        // Decode issues were seen previously with an AVR which had an internal
        // clock which was at 97% (3% slow).
        //
        // Tuning the intrabit parameter resolved the issue.
        //
        // Since each byte of serial is clocked independently, there will be no
        // cumulative error.
        //
        // In other words, we only have to be accurate for 8 bits.
        _rx_delay_intrabit *= _runningAtSpeedFactor;

        listen();
    }
    
    // This function sets the current object as the "listening"
    // one and returns true if it replaces another 
    bool listen()
    {
        if (!_rx_delay_stopbit)
        {
            return false;
        }

        _receive_buffer_head = _receive_buffer_tail = 0;


        pcied_.RegisterForPCIntEvent();

        return true;
    }

    void stopListening()
    {
        pcied_.DeRegisterForPCIntEvent();
    }

    size_t write(uint8_t b)
    {
        if (_tx_delay == 0) {
            return 0;
        }

        // By declaring these as local variables, the compiler will put them
        // in registers _before_ disabling interrupts and entering the
        // critical timing sections below, which makes it a lot easier to
        // verify the cycle timings
        volatile uint8_t *reg = _transmitPortRegister;
        uint8_t reg_mask = _transmitBitMask;
        uint8_t inv_mask = ~_transmitBitMask;
        uint8_t oldSREG = SREG;
        uint16_t delay = _tx_delay;

        cli();  // turn off interrupts for a clean txmit

        // Write the start bit
        *reg &= inv_mask;

        tunedDelay(delay);

        // Write each of the 8 bits
        for (uint8_t i = 8; i > 0; --i)
        {
            if (b & 1) // choose bit
            {
                *reg |= reg_mask; // send 1
            }
            else
            {
                *reg &= inv_mask; // send 0
            }

            tunedDelay(delay);
            b >>= 1;
        }

        // restore pin to natural state
        *reg |= reg_mask;

        SREG = oldSREG; // turn interrupts back on
        tunedDelay(_tx_delay);

        return 1;
    }



    size_t write(const uint8_t *buffer, size_t size)
    {
        size_t n = 0;

        while (size--)
        {
            if (write(*buffer++))
            {
                n++;
            }
            else
            {
                break;
            }
        }

        return n;
    }


    int read()
    {
        // Empty buffer?
        if (_receive_buffer_head == _receive_buffer_tail)
        {
            return -1;
        }

        // Read from "head"
        uint8_t d = _receive_buffer[_receive_buffer_head]; // grab next byte
        _receive_buffer_head = (_receive_buffer_head + 1) % BUF_SIZE;

        return d;
    }

    int available()
    {
        return (_receive_buffer_tail + BUF_SIZE - _receive_buffer_head) % BUF_SIZE;
    }

    inline void handle_interrupt() __attribute__((__always_inline__))
    {
        recv();
    }




public:

    static inline void tunedDelay(uint16_t delay) { 
        _delay_loop_2(delay);
    }

    inline void recv() __attribute__((__always_inline__))
    {
        // actually aiming for the front of the bit so we can read them all
        // sooner and exit the ISR sooner
        tunedDelay(_rx_delay_centering);

        // Read each of the 8 bits
        uint8_t d = 0;
        for (uint8_t i = 8; i > 0; --i)
        {
            d >>= 1;
            if (rx_pin_read())
            {
                d |= 0x80;
            }

            // only delay between bits, not after the last
            if (i - 1)
            {
                tunedDelay(_rx_delay_intrabit);
            }
        }

        // if buffer full, discard new byte
        uint8_t next = (_receive_buffer_tail + 1) % BUF_SIZE;
        if (next != _receive_buffer_head)
        {
            // save new data in buffer: tail points to where byte goes
            _receive_buffer[_receive_buffer_tail] = d; // save new byte
            _receive_buffer_tail = next;
        } 
    }


    uint8_t rx_pin_read()
    {
        return *_receivePortRegister & _receiveBitMask;
    }
    
    void setTX(uint8_t tx)
    {
        // First write, then set output. If we do this the other way around,
        // the pin would be output low for a short while before switching to
        // output high. Now, it is input with pullup for a short while, which
        // is fine. With inverse logic, either order is fine.
        digitalWrite(tx, HIGH);
        pinMode(tx, OUTPUT);
        _transmitBitMask = digitalPinToBitMask(tx);
        uint8_t port = digitalPinToPort(tx);
        _transmitPortRegister = portOutputRegister(port);
    }


    void setRX(uint8_t rx)
    {
        pinMode(rx, INPUT);
        digitalWrite(rx, HIGH);  // pullup for normal logic!
        _receivePin = rx;
        _receiveBitMask = digitalPinToBitMask(rx);
        uint8_t port = digitalPinToPort(rx);
        _receivePortRegister = portInputRegister(port);
    }


    // Return num - sub, or 1 if the result would be < 1
    static uint16_t subtract_cap(uint16_t num, uint16_t sub)
    {
        if (num > sub)
        {
            return num - sub;
        }
        else
        {
            return 1;
        }
    }


public:

    // It's the percent speed the clock is running at.
    // If 1.00, clock is running at actual time (100%).
    // If < 1.00, eg 0.97, clock is running at 97% speed (so it's slow).
    // If > 1.00, eg 1.03, clock is running at 103% speed (so it's fast).
    double _runningAtSpeedFactor;

    uint8_t _receivePin;
    uint8_t _receiveBitMask;
    volatile uint8_t *_receivePortRegister;
    uint8_t _transmitBitMask;
    volatile uint8_t *_transmitPortRegister;

    // Expressed as 4-cycle delays (must never be 0!)
    uint16_t _rx_delay_centering;
    uint16_t _rx_delay_intrabit;
    uint16_t _rx_delay_stopbit;
    uint16_t _tx_delay;

    uint8_t _receive_buffer[BUF_SIZE]; 
    volatile uint8_t _receive_buffer_tail;
    volatile uint8_t _receive_buffer_head;

    PCIntEventHandlerDelegate pcied_;
};


#endif  // ThinSoftwareSerial_h
