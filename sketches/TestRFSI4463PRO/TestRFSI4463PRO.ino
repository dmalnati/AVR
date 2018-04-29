// Some notes regarding implementation were taken from here:
// https://github.com/Yveaux/RadioHead/blob/master/RadioHead/RH_RF24.cpp
//
// Such as:
// - Waiting briefly to let SS pin go high after transfer
// - Using two transactions for command and reading of response
// - Power-on-Reset functionality being necessary above tying Shutdown to GND
//

#include <SPI.h>

#include "PAL.h"


static const uint32_t SPI_SPEED           = 1000000;
static const uint8_t  SPI_BIT_ORIENTATION = MSBFIRST;
static const uint8_t  SPI_MODE            = SPI_MODE0;

static const uint8_t PIN_SS = 15;

static const uint8_t PIN_SHUTDOWN = 14;




void PowerOnReset()
{
    uint32_t DELAY_MS = 50;
    
    PAL.PinMode(PIN_SHUTDOWN, OUTPUT);

    PAL.DigitalWrite(PIN_SHUTDOWN, LOW);
    PAL.Delay(DELAY_MS);
    
    PAL.DigitalWrite(PIN_SHUTDOWN, HIGH);
    PAL.Delay(DELAY_MS);
    
    PAL.DigitalWrite(PIN_SHUTDOWN, LOW);
    PAL.Delay(DELAY_MS);
}






void SendAndWaitAndReceive(uint8_t req, uint8_t *repBuf, uint8_t repBufLen)
{
    static const uint8_t READ_ATTEMPT_LIMIT = 100;
    
    SPI.beginTransaction(SPISettings(SPI_SPEED, SPI_BIT_ORIENTATION, SPI_MODE));
    PAL.DigitalWrite(PIN_SS, LOW);

    // Send req, then send READ_CMD_BUFF command, then clock in CTS
    SPI.transfer(req);

    PAL.DelayMicroseconds(50);
    PAL.DigitalWrite(PIN_SS, HIGH);
    SPI.endTransaction();

    
    
    uint8_t rep = 0x00; // temp value to make sure first loop iterates


    // Wait for CTS (clear to send) to equal 0xFF
    uint8_t readAttempts = 0;
    while (rep != 0xFF && readAttempts < READ_ATTEMPT_LIMIT)
    {
        ++readAttempts;
        
        SPI.beginTransaction(SPISettings(SPI_SPEED, SPI_BIT_ORIENTATION, SPI_MODE));
        PAL.DigitalWrite(PIN_SS, LOW);

        SPI.transfer(0x44);
        rep = SPI.transfer(0x00);

        if (rep == 0xFF)
        {
            // Consume as many bytes of response as are expected, could be zero
            for (uint16_t i = 0; i < repBufLen; ++i)
            {
                repBuf[i] = SPI.transfer(0x00);
            }
        }

        PAL.DelayMicroseconds(50);
        PAL.DigitalWrite(PIN_SS, HIGH);
        SPI.endTransaction();
    }
}



void DoReqRep(const char *op, uint8_t req, uint8_t repBufLen)
{
    uint8_t repBuf[repBufLen];
    memset(repBuf, 0, repBufLen);
    
    @fix@Serial.print(op);
    @fix@Serial.print(": ");
    @fix@Serial.print(req, HEX);
    @fix@Serial.print(" -> [");
    @fix@Serial.print(repBufLen);
    @fix@Serial.print("] ");

    SendAndWaitAndReceive(req, repBuf, repBufLen);

    for (uint16_t i = 0; i < repBufLen; ++i)
    {
        @fix@Serial.print(repBuf[i], HEX);
    }
    
    @fix@Serial.println();
}

void DoNOP()
{
    DoReqRep("NOP", 0x00, 0);
}

void DoPART_INFO()
{
    DoReqRep("PART_INFO", 0x01, 8);
}

void DoFUNC_INFO()
{
    DoReqRep("FUNC_INFO", 0x10, 6);
}


void DoGET_PH_STATUS()
{
    DoReqRep("GET_PH_STATUS", 0x21, 2);
}

void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    

    SPI.begin();


    

    PAL.PinMode(PIN_SS, OUTPUT);
    PAL.DigitalWrite(PIN_SS, HIGH);

    PowerOnReset();

    while (1)
    {
        @fix@Serial.println();
        @fix@Serial.println("Loop start");
        
        DoNOP();
        DoPART_INFO();
        DoFUNC_INFO();
        DoGET_PH_STATUS();

        

        PAL.Delay(1500);
    }
    
}


void loop() {}




