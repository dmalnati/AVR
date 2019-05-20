#include "PAL.h"
#include "Serial.h"


void Serial0::Start(uint32_t baud)
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
        uint16_t UBRR = (uint16_t)((PAL.GetOscillatorFreq() / 16 / baud) - 1);
        
        // Modified example init from pdf p. 176
        
        /* Set baud rate */
        UBRR0H = (uint8_t)(UBRR >> 8);
        UBRR0L = (uint8_t)UBRR;
        
        /* Enable receiver ISR, receiver, and transmitter */
        UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
        
        /* Set frame format: 8N1 - 8data, no parity, 1stop bit */
        UCSR0C = (0 << UCSZ02) | (1 << UCSZ01) | (1 << UCSZ00);
    }
}

void Serial0::Stop()
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
         
        /* 
         * If the Receiver is disabled (RXENn = 0), the receive buffer will
         * be flushed and consequently the RXCn bit will become zero
         */
        
        // So no need to check for and discard unread data, it's automatic
        
        // Disable RX / TX
        // Notably all other bits are appropriate to set to 0 here as well.
        // This includes disabling the interrupt-enable bit for RX.
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
        
        // Cut power to USART
        PAL.PowerDownSerial0();
    }
}


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

Queue<uint8_t, Serial0::BUF_RX_SIZE> Serial0::rxQ_;


Serial0 S0;


ISR(USART_RX_vect)
{
    // check if error bits are set
    // uint8_t frameOk  = (UCSR0A & (1 << FE0))  ? 0 : 1;
    // uint8_t dorOk    = (UCSR0A & (1 << DOR0)) ? 0 : 1;
    // uint8_t parityOk = (UCSR0A & (1 << UPE0)) ? 0 : 1;
    
    uint8_t val = UDR0;
    
    // if (!frameOk)
    // {
        // val = 'f';
    // }
    // if (!dorOk)
    // {
        // val = 'd';
    // }
    // if (!parityOk)
    // {
        // val = 'p';
    // }
    
    if (!Serial0::rxQ_.Push(val))
    {
        uint8_t tmpVal;
        
        Serial0::rxQ_.Pop(tmpVal);
        Serial0::rxQ_.Push(val);
    }
}




