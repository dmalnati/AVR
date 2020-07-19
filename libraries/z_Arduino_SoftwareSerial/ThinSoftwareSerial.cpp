#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <ThinSoftwareSerial.h>
#include <util/delay_basic.h>


ThinSoftwareSerial *ThinSoftwareSerial::active_object = 0;
uint8_t ThinSoftwareSerial::_receive_buffer[_SS_MAX_RX_BUFF]; 
volatile uint8_t ThinSoftwareSerial::_receive_buffer_tail = 0;
volatile uint8_t ThinSoftwareSerial::_receive_buffer_head = 0;


ISR(PCINT0_vect)
{
  ThinSoftwareSerial::handle_interrupt();
}
ISR(PCINT1_vect, ISR_ALIASOF(PCINT0_vect));
ISR(PCINT2_vect, ISR_ALIASOF(PCINT0_vect));






