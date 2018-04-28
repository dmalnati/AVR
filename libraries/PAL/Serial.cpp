#include "Serial.h"


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




