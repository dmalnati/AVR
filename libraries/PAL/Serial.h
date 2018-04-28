#ifndef __SERIAL_H__
#define __SERIAL_H__


class Serial0
{
public:

    void Start(uint32_t baud)
    {
        /*        
         * The USART has to be initialized before any communication can take
         * place.
         * 
         * The initialization process normally consists of setting the
         * baud rate, setting frame format and enabling the Transmitter or the
         * Receiver depending on the usage. 
         * 
         * For interrupt driven USART operation, the Global Interrupt Flag
         * should be cleared (and interrupts globally disabled) when doing the
         * initialization.     
         */

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Stop any ongoing operation, leave in known good inert state
            Stop();
            
            // Power up USART
            PAL.PowerUpSerial0();
            
            // Calculate register values for given baud according to spec
            uint16_t UBRR = (uint16_t)((PAL.GetOscillatorFreq() / 16 / baud) + 1);
        
            // Modified example init from pdf p. 176
            
            /* Set baud rate */
            UBRR0H = (uint8_t)(UBRR >> 8);
            UBRR0L = (uint8_t)UBRR;
            
            /* Enable receiver and transmitter */
            UCSR0B = (1 << RXEN0) | (1 << TXEN0);
            
            /* Set frame format: 8data, 1stop bit */
            UCSR0C = (3 << UCSZ00);
        }
    }
    
    void Stop()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            /*
             * Before doing a re-initialization with changed baud rate or frame
             * format, be sure that there are no ongoing transmissions during the
             * period the registers are changed. The TXCn Flag can be used to check
             * that the Transmitter has completed all transfers, and the RXC Flag
             * can be used to check that there are no unread data in the receive
             * buffer. Note that the TXCn Flag must be cleared before each
             * transmission (before UDRn is written) if it is used for this purpose.
             */
            
            /* 
             * The disabling of the Transmitter (setting the TXEN to zero) will
             * not become effective until ongoing and pending transmissions are
             * completed, i.e., when the Transmit Shift Register and
             * Transmit Buffer Register do not contain data to be transmitted. 
             */
            
            // So just disable right away
            
            // Disable RX / TX
            // Notably all other bits are appropriate to set to 0 here as well.
            UCSR0B = 0;
            
            
            // Check any ongoing transfers complete, wait for completion.
            //
            // This handles the situation where the shift-register is still
            // clocking out a byte copied from the buffer.
            // 
            // Not obviously required, but does leave the serial interface in
            // a well-known state, and at basically no cost, is worth doing.
            if (hasWrittenAtLeastOneByte_)
            {
                while (!(UCSR0A & (1 << TXC0)));
            }
            
            
            
            
            
            // check if any unread data and discard

            // Clear interrupts for RX
            
            
            
            // Cut power to USART
            PAL.PowerDownSerial0();
        }
        
    }
    
    void Write(uint8_t b)
    {
        // Synchronous.
        // Don't allow function to return until byte fully transmitted.
        //
        // No need to check if data is already in the buffer, this is the only
        // function which can put data there, and we're ensuring it's gone by
        // the time we return.
        //
        // The buffer, UDR0, is not the buffer the USART uses to transmit from.
        // The USART uses a shift register, which loads from the UDR0 when it is
        // ready to send a new byte.
        // 
        // So waiting until the UDR0 is empty means that the shift register has
        // taken a copy of its value and is sending it (or has sent it).
        //
        // So in terms of streamlining transmissions, this should be just fine
        // waiting for synchronous write, as the next byte can be loaded into
        // UDR0 while the shift register is working the prior.
        
        
        // Clear the TXCn flag by writing 1 to it.
        // The last two bits are the only other writeable bits, and are
        // appropriate to set to 0.
        //
        // This supports the ability to wait for final data to be shifted out of
        // register if Stop is called before complete.
        UCSR0A = 0b01000000;
        
        // Put data into buffer leading to shift register
        UDR0 = b;
        
        // Wait for this data to be put into the shift register and now be empty
        while (!(UCSR0A & (1 << UDRE0)));
        
        // Keep track of state for disambiguation later
        hasWrittenAtLeastOneByte_ = 1;
    }
    
    void Write(uint8_t *buf, uint16_t bufSize)
    {
        for (uint16_t i = 0; i < bufSize; ++i)
        {
            Write(buf[i]);
        }
    }
    
    void Write(const char *str)
    {
        uint16_t bufSize = strlen(str);
        
        Write((uint8_t *)str, bufSize);
    }

private:

    static uint8_t hasWrittenAtLeastOneByte_;
};

// Necessary to keep some state.
// Impossible to tell initial state from one where tx isn't complete yet
// when you want to stop TX.
// https://www.avrfreaks.net/forum/uart-udrie-txcie
//
// Action    Flags         Comment
// Startup   UDRE=1 TXC=0                                      [this state]
// Load UDR  UDRE=0 TXC=0  Software loads UDR
// UDR->SR   UDRE=1 TXC=0  Hardware loads Shift Register
// LOAD UDR  UDRE=0 TXC=0  Software loads UDR
// 
// after first byte finishes shifting
// UDR->SR   UDRE=1 TXC=0  Hardware Loads Shift Register       [this state]
// 
// after second byte finishes shifting
// nothing   UDRE=1 TXC=1
//
//
// We can disambiguate by knowing if we've ever written before.
uint8_t Serial0::hasWrittenAtLeastOneByte_ = 0;


#endif  // __SERIAL_H__


























