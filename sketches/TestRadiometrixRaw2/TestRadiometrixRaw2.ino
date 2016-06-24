/*
Demo Code to Drive NTX2B via PWM
 
Written by Anthony Stirk M0UPU
RTTY code from Rob Harrison Icarus Project.
 
http://ukhas.org.uk
 
This example code is in the public domain.
*/
 
#define RADIOPIN 9
 
#include <string.h>
#include <util/crc16.h>
 
char datastring[80];
 
void setup() {
    pinMode(RADIOPIN,OUTPUT);
    setPwmFrequency(RADIOPIN, 1);
}
 
void loop() {
    snprintf(datastring,80,"RTTY TEST BEACON RTTY TEST BEACON"); // Puts the text in the datastring
    unsigned int CHECKSUM = gps_CRC16_checksum(datastring); // Calculates the checksum for this datastring
    char checksum_str[6];
    sprintf(checksum_str, "*%04X\n", CHECKSUM);
    strcat(datastring,checksum_str);
    rtty_txstring (datastring);
}
 
void rtty_txstring (char * string) {
    /* Simple function to sent a char at a time to
    ** rtty_txbyte function.
    ** NB Each char is one byte (8 Bits)
    */
 
    char c;
    c = *string++;
 
    while ( c != '\0') {
        rtty_txbyte (c);
        c = *string++;
    }
}
void rtty_txbyte (char c) {
    /* Simple function to sent each bit of a char to
    ** rtty_txbit function.
    ** NB The bits are sent Least Significant Bit first
    **
    ** All chars should be preceded with a 0 and
    ** proceed with a 1. 0 = Start bit; 1 = Stop bit
    **
    */
 
    int i;
 
    rtty_txbit (0); // Start bit
 
    // Send bits for for char LSB first
 
    for (i=0;i<7;i++) { // Change this here 7 or 8 for ASCII-7 / ASCII-8
        if (c & 1) rtty_txbit(1);
        else rtty_txbit(0);
 
        c = c >> 1;
    }
    rtty_txbit (1); // Stop bit
    rtty_txbit (1); // Stop bit
}
 
void rtty_txbit (int bit) {

    static const uint16_t BELL_202_TONE_MARK  = 1200;
    static const uint16_t BELL_202_TONE_SPACE = 2200;
    
    if (bit) {
        // high
        //analogWrite(RADIOPIN,110);
        //analogWrite(RADIOPIN,150);
        tone(RADIOPIN, BELL_202_TONE_MARK);
    }
    else {
        // low
        //analogWrite(RADIOPIN,110);
        //analogWrite(RADIOPIN,225);
        tone(RADIOPIN, BELL_202_TONE_SPACE);
    }
 
    // delayMicroseconds(3370); // 300 baud
    delayMicroseconds(10000); // For 50 Baud uncomment this and the line below.
    delayMicroseconds(10150); // You can't do 20150 it just doesn't work as the
    // largest value that will produce an accurate delay is 16383
    // See : http://arduino.cc/en/Reference/DelayMicroseconds
}
 
uint16_t gps_CRC16_checksum (char *string) {
    size_t i;
    uint16_t crc;
    uint8_t c;
 
    crc = 0xFFFF;
 
    // Calculate checksum ignoring the first two $s
    for (i = 2; i < strlen(string); i++) {
        c = string[i];
        crc = _crc_xmodem_update (crc, c);
    }
 
    return crc;
}
 
void setPwmFrequency(int pin, int divisor) {
    byte mode;
    if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
        switch(divisor) {
            case 1:
                mode = 0x01;
                break;
            case 8:
                mode = 0x02;
                break;
            case 64:
                mode = 0x03;
                break;
            case 256:
                mode = 0x04;
                break;
            case 1024:
                mode = 0x05;
                break;
            default:
                return;
        }
 
        if(pin == 5 || pin == 6) {
            TCCR0B = TCCR0B & 0b11111000 | mode;
        }
        else {
            TCCR1B = TCCR1B & 0b11111000 | mode;
        }
    }
    else if(pin == 3 || pin == 11) {
        switch(divisor) {
            case 1:
                mode = 0x01;
                break;
            case 8:
                mode = 0x02;
                break;
            case 32:
                mode = 0x03;
                break;
            case 64:
                mode = 0x04;
                break;
            case 128:
                mode = 0x05;
                break;
            case 256:
                mode = 0x06;
                break;
            case 1024:
                mode = 0x7;
                break;
            default:
                return;
        }
        TCCR2B = TCCR2B & 0b11111000 | mode;
    }
}
